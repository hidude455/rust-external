#include "FeatureManager.h"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace {
    std::string TrimCopy(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
        return s.substr(start, end - start);
    }

    std::string ColorToString(uint32_t color) {
        std::ostringstream oss;
        oss << "0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << color;
        return oss.str();
    }

    bool ParseColor(const std::string& token, uint32_t& out) {
        std::string trimmed = TrimCopy(token);
        if (trimmed.size() > 2 && (trimmed[0] == '0' && (trimmed[1] == 'x' || trimmed[1] == 'X'))) {
            trimmed = trimmed.substr(2);
        }
        if (trimmed.empty()) return false;
        try {
            out = static_cast<uint32_t>(std::stoul(trimmed, nullptr, 16));
            return true;
        } catch (...) {
            return false;
        }
    }
}

namespace Features {

    CFeatureManager::CFeatureManager() : m_memory(nullptr), m_currentTarget(nullptr),
        m_d3dDevice(nullptr), m_vs(nullptr), m_ps(nullptr), m_inputLayout(nullptr),
        m_vb(nullptr), m_ib(nullptr), m_renderInit(false) {
        std::random_device rd; m_rng = std::mt19937(rd());
    }

    CFeatureManager::~CFeatureManager() { Shutdown(); }

    bool CFeatureManager::Initialize(Memory::CGameMemory* memory) {
        m_memory = memory;
        return m_memory != nullptr;
    }

    void CFeatureManager::Shutdown() {
        if(m_vs){m_vs->Release();m_vs=nullptr;}
        if(m_ps){m_ps->Release();m_ps=nullptr;}
        if(m_inputLayout){m_inputLayout->Release();m_inputLayout=nullptr;}
        if(m_vb){m_vb->Release();m_vb=nullptr;}
        if(m_ib){m_ib->Release();m_ib=nullptr;}
        if(m_blendState){m_blendState->Release();m_blendState=nullptr;}
        m_renderInit=false;
        m_memory = nullptr;
        m_currentTarget = nullptr;
    }

    void CFeatureManager::Update() {
        if (!m_memory || !m_memory->IsAttached()) return;
        m_memory->Update();
        if (!m_memory->IsInGame()) return;
        ApplyWorldExploits(); ApplyWeaponMods(); ApplyMovement();
        if (!m_aimbotCfg.enabled) return;
        bool keyOK = true;
        if (m_aimbotCfg.aimKey.key) {
            if (m_aimbotCfg.aimKey.mode == KeybindMode::Hold) keyOK = GetAsyncKeyState(m_aimbotCfg.aimKey.key) & 0x8000;
            else keyOK = m_aimbotCfg.aimKey.enabled;
        }
        if (!keyOK) return;
        m_currentTarget = SelectAimbotTarget();
        if (!m_currentTarget) return;
        Memory::Vector3 tp = m_currentTarget->position;
        switch (m_aimbotCfg.targetBone) {
        case TargetBone::Head: tp.y += 1.7f; break;
        case TargetBone::Neck: tp.y += 1.5f; break;
        case TargetBone::Chest: tp.y += 1.2f; break;
        case TargetBone::Pelvis: tp.y += 0.8f; break;
        case TargetBone::LeftFoot: case TargetBone::RightFoot: tp.y += 0.1f; break;
        case TargetBone::Random: { std::uniform_int_distribution<int> d(0,5); float o[]={1.7f,1.5f,1.2f,0.8f,0.1f,0.1f}; tp.y+=o[d(m_rng)]; break; }
        }
        if (m_aimbotCfg.predictionTime > 0) tp = PredictPosition(*m_currentTarget, m_aimbotCfg.predictionTime);
        Memory::Vector2 aa = CalculateAimAngles(m_memory->GetLocalPlayer().position, tp);
        if (m_aimbotCfg.hitchance > 0 && CalculateHitchance(*m_currentTarget, aa) < m_aimbotCfg.hitchance) return;
        if (m_aimbotCfg.silentAim) ApplySilentAim(aa);
        else if (m_aimbotCfg.memoryAim) ApplyMemoryAim(aa);
        else ApplyAimbot(aa);
        if (m_aimbotCfg.autoFire) { mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0); Sleep(50); mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0); }
    }

    Memory::GameEntity* CFeatureManager::SelectAimbotTarget() {
        const auto& local = m_memory->GetLocalPlayer();
        const auto& cam = m_memory->GetCamera();
        Memory::GameEntity* best = nullptr; float bestFOV = m_aimbotCfg.fov;
        for (const auto& e : m_memory->GetEntities()) {
            if (e.address == local.address) continue;
            if (e.isSleeping && m_aimbotCfg.ignoreSleepers) continue;
            if (e.isWounded && m_aimbotCfg.ignoreWounded) continue;
            if (e.isTeammate && m_aimbotCfg.ignoreTeammates) continue;
            if (e.distance > m_aimbotCfg.maxDistance) continue;
            bool vt = false;
            switch (e.type) {
            case Memory::EntityType::Player: vt=m_aimbotCfg.targetPlayers; break;
            case Memory::EntityType::Scientist: vt=m_aimbotCfg.targetNPCs; break;
            case Memory::EntityType::Animal: vt=m_aimbotCfg.targetAnimals; break;
            }
            if (!vt) continue;
            if (m_aimbotCfg.visibilityCheck && !e.isVisible) continue;
            float fov = GetFOV(cam.viewAngles, CalculateAimAngles(local.position, e.position));
            if (fov < bestFOV) { bestFOV = fov; best = const_cast<Memory::GameEntity*>(&e); }
        }
        return best;
    }

    Memory::Vector2 CFeatureManager::CalculateAimAngles(const Memory::Vector3& from, const Memory::Vector3& to) {
        Memory::Vector3 delta = { to.x - from.x, to.y - from.y, to.z - from.z };
        float distance = sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

        Memory::Vector2 angles;
        angles.x = -atan2f(delta.x, delta.z) * (180.0f / 3.14159265359f);
        angles.y = atan2f(delta.y, distance) * (180.0f / 3.14159265359f);
        return angles;
    }

    Memory::Vector3 CFeatureManager::PredictPosition(const Memory::GameEntity& target, float time) {
        Memory::Vector3 predicted = target.position;
        predicted.x += target.velocity.x * time;
        predicted.y += target.velocity.y * time;
        predicted.z += target.velocity.z * time;
        return predicted;
    }

    float CFeatureManager::GetFOV(const Memory::Vector2& viewAngles, const Memory::Vector2& targetAngles) {
        float dx = targetAngles.x - viewAngles.x;
        float dy = targetAngles.y - viewAngles.y;
        return sqrtf(dx * dx + dy * dy);
    }

    void CFeatureManager::ApplyAimbot(const Memory::Vector2& aimAngles) {
        const auto& camera = m_memory->GetCamera();
        Memory::Vector2 delta = {
            aimAngles.x - camera.viewAngles.x,
            aimAngles.y - camera.viewAngles.y
        };

        if (m_aimbotCfg.humanizeAim) {
            std::uniform_real_distribution<float> jitter(-m_aimbotCfg.aimJitter, m_aimbotCfg.aimJitter);
            delta.x += jitter(m_rng);
            delta.y += jitter(m_rng);
        }

        float smooth = m_aimbotCfg.smoothness;
        delta.x /= smooth;
        delta.y /= smooth;

        mouse_event(MOUSEEVENTF_MOVE, (DWORD)delta.x, (DWORD)delta.y, 0, 0);
    }

    float CFeatureManager::CalculateHitchance(const Memory::GameEntity& t, const Memory::Vector2&) {
        float d=t.distance; if(d<10)return 100; if(d>400)return 30;
        float bc=100-(d/400)*70; if(t.velocity.x||t.velocity.z)bc-=15; return std::max(0.0f,bc);
    }

    void CFeatureManager::ApplySilentAim(const Memory::Vector2& aa) {
        uint64_t a=m_memory->GetBaseAddress()+0x31F0058; if(!a)return;
        a=m_memory->Read<uint64_t>(a); if(!a)return;
        a=m_memory->Read<uint64_t>(a+0xB8); if(!a)return;
        a=m_memory->Read<uint64_t>(a); if(a) m_memory->Write<Memory::Vector2>(a+0x2E4,aa);
    }

    void CFeatureManager::ApplyMemoryAim(const Memory::Vector2& aa) { ApplySilentAim(aa); }

    void CFeatureManager::Render(ID3D11DeviceContext* ctx, int w, int h) {
        if(!m_memory||!m_memory->IsAttached()||!m_memory->IsInGame())return;
        if(!m_renderInit){
            ID3D11Device* dev=nullptr;ctx->GetDevice(&dev);
            if(dev){InitRenderResources(dev);dev->Release();}
            if(!m_renderInit)return;
        }
        BeginBatch(ctx);
        if(m_aimbotCfg.fovCircle)RenderFOVCircle(ctx,w,h);
        if(m_aimbotCfg.targetLine&&m_currentTarget)RenderTargetLine(ctx,w,h);
        if(m_aimbotCfg.showPrediction&&m_currentTarget)RenderPrediction(ctx,w,h);
        if(m_aimbotCfg.bulletTracers)RenderTracers(ctx,w,h);
        if(m_aimbotCfg.highlightTarget&&m_currentTarget)RenderHighlight(ctx,w,h);
        if(m_visualsCfg.enabled){
            for(const auto& e:m_memory->GetEntities()){
                if(e.address==m_memory->GetLocalPlayer().address)continue;
                if(e.distance>m_visualsCfg.maxDistance)continue;
                if(m_visualsCfg.visibilityCheck&&!e.isVisible)continue;
                switch(e.type){
                case Memory::EntityType::Player: case Memory::EntityType::Scientist:
                    if((e.type==Memory::EntityType::Player&&m_visualsCfg.showPlayers)||(e.type==Memory::EntityType::Scientist&&m_visualsCfg.showNPCs))
                        RenderPlayerESP(ctx,e,w,h);
                    break;
                default:
                    if(m_worldVisualsCfg.enabled){
                        const WorldVisualsConfig::ItemVisual* c=nullptr;
                        switch(e.type){
                        case Memory::EntityType::Ore:c=&m_worldVisualsCfg.allOres;break;
                        case Memory::EntityType::Collectible:c=&m_worldVisualsCfg.collectables;break;
                        case Memory::EntityType::LootContainer:c=&m_worldVisualsCfg.multiTierCrates;break;
                        case Memory::EntityType::Stash:c=&m_worldVisualsCfg.stashes;break;
                        case Memory::EntityType::Corpse:c=&m_worldVisualsCfg.corpses;break;
                        case Memory::EntityType::Backpack:c=&m_worldVisualsCfg.backpacks;break;
                        case Memory::EntityType::SupplyDrop:c=&m_worldVisualsCfg.supplyDrops;break;
                        case Memory::EntityType::HackableCrate:c=&m_worldVisualsCfg.hackableCrates;break;
                        case Memory::EntityType::Helicopter:c=&m_worldVisualsCfg.patrolHelicopter;break;
                        case Memory::EntityType::BradleyAPC:c=&m_worldVisualsCfg.bradleyAPC;break;
                        case Memory::EntityType::CargoShip:c=&m_worldVisualsCfg.cargoShip;break;
                        case Memory::EntityType::LockedCrate:c=&m_worldVisualsCfg.lockedCrates;break;
                        case Memory::EntityType::Vehicle:c=&m_worldVisualsCfg.vehicles;break;
                        case Memory::EntityType::Deployable:c=&m_worldVisualsCfg.deployables;break;
                        case Memory::EntityType::Animal:c=&m_worldVisualsCfg.allAnimals;break;
                        case Memory::EntityType::ToolCupboard:c=&m_worldVisualsCfg.toolCupboard;break;
                        case Memory::EntityType::OilRig:c=&m_worldVisualsCfg.oilRig;break;
                        }
                        if(c&&c->enabled&&e.distance<=c->maxDistance)RenderItemESP(ctx,e,w,h,*c);
                    }
                }
            }
        }
        if(m_uiCfg.customCrosshairs)RenderCrosshair(ctx,w,h);
        FlushBatch(ctx);
        if(m_uiCfg.showWatermark)RenderWatermark(ctx,w,h);
    }

    void CFeatureManager::RenderPlayerESP(ID3D11DeviceContext* ctx, const Memory::GameEntity& e, int w, int h) {
        Memory::Vector2 sp, sh; Memory::Vector3 hp=e.position; hp.y+=1.8f;
        if(!m_memory->WorldToScreen(e.position,sp,w,h))return;
        if(!m_memory->WorldToScreen(hp,sh,w,h))return;
        float bh=sp.y-sh.y, bw=bh*0.4f, x=sh.x-bw/2, y=sh.y;
        bool isTarget = m_currentTarget && e.address==m_currentTarget->address;
        uint32_t col = GetEntityColor(e);
        if(m_visualsCfg.customESPColor){
            col = m_visualsCfg.customESPColorValue;
        }
        if(m_visualsCfg.highlightESP && isTarget){
            col = m_visualsCfg.highlightESPColor;
        }
        if(isTarget && m_visualsCfg.highlightESP){
            float hx=x-6.0f, hy=y-6.0f, hw=bw+12.0f, hh=bh+12.0f;
            uint32_t fillColor=(m_visualsCfg.highlightESPColor&0x00FFFFFF)|0x20000000;
            DrawFilledRect(ctx,hx,hy,hw,hh,fillColor);
            DrawBox(ctx,hx,hy,hw,hh,m_visualsCfg.highlightESPColor,2.0f);
        }
        if(m_visualsCfg.chams)RenderChams(ctx,e,w,h);
        switch(m_visualsCfg.boxStyle){
        case ESPBoxStyle::Box2D:DrawBox(ctx,x,y,bw,bh,col,m_visualsCfg.boxThickness);break;
        case ESPBoxStyle::Corner:DrawCornerBox(ctx,x,y,bw,bh,col,m_visualsCfg.boxThickness,bh*m_visualsCfg.cornerLength);break;
        case ESPBoxStyle::Filled:{uint32_t fc=(col&0x00FFFFFF)|0x40000000;DrawFilledRect(ctx,x,y,bw,bh,fc);DrawCornerBox(ctx,x,y,bw,bh,col,m_visualsCfg.boxThickness,bh*m_visualsCfg.cornerLength);break;}
        }
        if(m_visualsCfg.skeletons)RenderSkeleton(ctx,e,w,h);
        if(m_visualsCfg.targetBoneESP && isTarget){
            uint32_t prevColor=m_visualsCfg.skeletonColor;
            float prevThickness=m_visualsCfg.skeletonThickness;
            uint32_t targetColor=m_visualsCfg.highlightESP?m_visualsCfg.highlightESPColor:prevColor;
            float targetThickness=std::max(prevThickness,2.5f);
            m_visualsCfg.skeletonColor=targetColor;
            m_visualsCfg.skeletonThickness=targetThickness;
            RenderSkeleton(ctx,e,w,h);
            m_visualsCfg.skeletonColor=prevColor;
            m_visualsCfg.skeletonThickness=prevThickness;
        }
        if(m_visualsCfg.viewDirectionArrow)RenderViewArrow(ctx,e,w,h);
        if(m_visualsCfg.offscreenArrows)RenderOffscreenArrow(ctx,e,w,h);
        float iy=y-2;
        if(m_visualsCfg.username&&!e.name.empty()){DrawText(ctx,e.name,sp.x,iy,m_visualsCfg.usernameColor,true,1.0f);iy-=14;}
        if(m_visualsCfg.distance){char b[32];sprintf_s(b,"%.0fm",e.distance);DrawText(ctx,b,sp.x,iy,m_visualsCfg.distanceColor,true,1.0f);iy-=14;}
        if(m_visualsCfg.heldItem&&!e.heldItemName.empty()){DrawText(ctx,e.heldItemName,sp.x,iy,m_visualsCfg.heldItemColor,true,1.0f);iy-=14;}
        if(m_visualsCfg.insideBuilding&&e.isInsideBuilding){DrawText(ctx,"INDOORS",sp.x,iy,m_visualsCfg.insideBuildingColor,true,1.0f);}
        if(m_visualsCfg.tracers && isTarget)DrawLine(ctx,(float)w/2,(float)h,sp.x,y+bh,m_visualsCfg.tracerLineColor,1.8f);
        if(m_visualsCfg.showHeadDot)DrawCircle(ctx,sh.x,sh.y,m_visualsCfg.headDotSize,m_visualsCfg.headDotColor,1.0f,true);
        if(m_visualsCfg.hotbar)RenderHotbar(ctx,e,w,h);
        if(m_visualsCfg.inventoryOverlay && m_currentTarget && e.address==m_currentTarget->address)RenderInventoryOverlay(ctx,e,w,h);
        if(m_visualsCfg.steamAvatar)RenderAvatar(ctx,e,w,h);
    }

    void CFeatureManager::RenderSkeleton(ID3D11DeviceContext* ctx, const Memory::GameEntity& e, int w, int h) {
        int bp[][2]={{0,1},{1,2},{2,3},{3,4},{4,5},{0,6},{6,7},{7,8},{8,9},{0,10},{10,11},{11,12},{12,13},{13,14},{14,15},{12,16},{16,17},{17,18},{12,19},{19,20},{20,21}};
        int nb=m_visualsCfg.fullBodySkeleton?22:16;
        for(int i=0;i<nb;i++){Memory::Vector2 s1,s2;if(m_memory->WorldToScreen(e.bonePositions[bp[i][0]],s1,w,h)&&m_memory->WorldToScreen(e.bonePositions[bp[i][1]],s2,w,h))DrawLine(ctx,s1.x,s1.y,s2.x,s2.y,m_visualsCfg.skeletonColor,m_visualsCfg.skeletonThickness);}
    }
    void CFeatureManager::RenderChams(ID3D11DeviceContext* ctx, const Memory::GameEntity& e, int w, int h) {
        Memory::Vector2 sp; if(!m_memory->WorldToScreen(e.position,sp,w,h))return;
        DrawCircle(ctx,sp.x,sp.y,30.0f,m_visualsCfg.chamColor,2.0f,true);
    }
    void CFeatureManager::RenderViewArrow(ID3D11DeviceContext* ctx, const Memory::GameEntity& e, int w, int h) {
        Memory::Vector2 sp; if(!m_memory->WorldToScreen(e.position,sp,w,h))return;
        float a=atan2f(e.velocity.x,e.velocity.z);
        DrawLine(ctx,sp.x,sp.y,sp.x+cosf(a)*m_visualsCfg.arrowSize,sp.y+sinf(a)*m_visualsCfg.arrowSize,m_visualsCfg.arrowColor,1.5f);
    }
    void CFeatureManager::RenderOffscreenArrow(ID3D11DeviceContext* ctx, const Memory::GameEntity& e, int w, int h) {
        Memory::Vector2 sp; if(m_memory->WorldToScreen(e.position,sp,w,h))return;
        float cx=(float)w/2,cy=(float)h/2,a=atan2f(e.position.x-m_memory->GetLocalPlayer().position.x,e.position.z-m_memory->GetLocalPlayer().position.z);
        float m=m_visualsCfg.offscreenDistance,ax=cx+cosf(a)*(cx-m),ay=cy+sinf(a)*(cy-m);
        ax=std::max(m,std::min((float)w-m,ax)); ay=std::max(m,std::min((float)h-m,ay));
        float s=m_visualsCfg.offscreenSize; DrawTriangle(ctx,ax-s,ay-s,ax+s,ay-s,ax,ay+s,m_visualsCfg.offscreenColor,true);
    }
    void CFeatureManager::RenderHotbar(ID3D11DeviceContext* ctx, const Memory::GameEntity& e, int w, int h) {
        Memory::Vector2 sp; if(!m_memory->WorldToScreen(e.position,sp,w,h))return;
        float sx=sp.x-(e.hotbarItems.size()*12.0f)/2, sy=sp.y+20;
        for(size_t i=0;i<e.hotbarItems.size();i++){
            DrawFilledRect(ctx,sx+i*12,sy,10,10,0x80000000);
            DrawBox(ctx,sx+i*12,sy,10,10,0xFFFFFFFF,1.0f);
        }
    }
    void CFeatureManager::RenderInventoryOverlay(ID3D11DeviceContext* ctx, const Memory::GameEntity& e, int w, int h) {
        if(e.hotbarItems.empty() && e.attachments.empty()) return;
        Memory::Vector2 sp; if(!m_memory->WorldToScreen(e.position,sp,w,h))return;
        float baseX = sp.x;
        float baseY = sp.y - 120.0f;
        DrawFilledRect(ctx,baseX-90,baseY-28,180,24,0xA0000000);
        DrawBox(ctx,baseX-90,baseY-28,180,24,0xFF30FF30,1.0f);
        DrawText(ctx,"Inventory",baseX,baseY-20,0xFFFFFFFF,true,1.0f);

        float itemY = baseY - 2;
        if(!e.hotbarItems.empty()){
            DrawText(ctx,"Hotbar",baseX-80,itemY,0xFFB0FFD0,false,0.9f);
            itemY += 14;
            for(const auto& item : e.hotbarItems){
                DrawText(ctx,item,baseX-70,itemY,0xFFFFFFFF,false,0.9f);
                itemY += 14;
            }
        }
        if(!e.attachments.empty()){
            itemY += 6;
            DrawText(ctx,"Attachments",baseX-80,itemY,0xFFD0FFB0,false,0.9f);
            itemY += 14;
            for(const auto& att : e.attachments){
                DrawText(ctx,att,baseX-70,itemY,0xFFFFFFFF,false,0.9f);
                itemY += 14;
            }
        }
    }
    void CFeatureManager::RenderAvatar(ID3D11DeviceContext* ctx, const Memory::GameEntity& e, int w, int h) {
        Memory::Vector2 sp; if(!m_memory->WorldToScreen(e.position,sp,w,h))return;
        float s=m_visualsCfg.avatarSize; DrawFilledRect(ctx,sp.x-s/2,sp.y-s-40,s,s,0xFF404040);DrawBox(ctx,sp.x-s/2,sp.y-s-40,s,s,0xFFFFFFFF,1.0f);
    }
    void CFeatureManager::RenderItemESP(ID3D11DeviceContext* ctx, const Memory::GameEntity& e, int w, int h, const WorldVisualsConfig::ItemVisual& c) {
        Memory::Vector2 sp; if(!m_memory->WorldToScreen(e.position,sp,w,h))return;
        if(c.box)DrawCornerBox(ctx,sp.x-10,sp.y-10,20,20,c.color,1.0f,5.0f);
        if(c.chams)DrawCircle(ctx,sp.x,sp.y,15.0f,c.chamColor,2.0f,true);
        if(!e.name.empty())DrawText(ctx,e.name,sp.x,sp.y-16,c.color,true,1.0f);
        char b[32];sprintf_s(b,"%.0fm",e.distance);DrawText(ctx,b,sp.x,sp.y+4,c.color,true,1.0f);
    }
    void CFeatureManager::RenderHighlight(ID3D11DeviceContext* ctx, int w, int h) {
        if(!m_currentTarget)return; Memory::Vector2 sp;
        if(m_memory->WorldToScreen(m_currentTarget->position,sp,w,h))DrawCircle(ctx,sp.x,sp.y,25.0f,m_aimbotCfg.highlightColor,3.0f,false);
    }
    void CFeatureManager::RenderTracers(ID3D11DeviceContext* ctx, int w, int h) {
        auto now=std::chrono::steady_clock::now();
        m_tracers.erase(std::remove_if(m_tracers.begin(),m_tracers.end(),[&](auto&t){return std::chrono::duration<float>(now-t.second).count()>m_aimbotCfg.tracerDuration;}),m_tracers.end());
        for(auto&[p,_]:m_tracers)DrawCircle(ctx,p.x,p.y,3.0f,m_aimbotCfg.tracerColor,1.0f,true);
    }
    void CFeatureManager::RenderCrosshair(ID3D11DeviceContext* ctx, int w, int h) {
        float cx=(float)w/2,cy=(float)h/2,sz=m_uiCfg.crosshairSize,g=m_uiCfg.crosshairGap,t=m_uiCfg.crosshairThickness;uint32_t c=m_uiCfg.crosshairColor;
        switch(m_uiCfg.crosshairStyle){
        case CrosshairStyle::Cross:DrawLine(ctx,cx-sz-g,cy,cx-g,cy,c,t);DrawLine(ctx,cx+g,cy,cx+sz+g,cy,c,t);DrawLine(ctx,cx,cy-sz-g,cx,cy-g,c,t);DrawLine(ctx,cx,cy+g,cx,cy+sz+g,c,t);break;
        case CrosshairStyle::Dot:DrawCircle(ctx,cx,cy,sz/2,c,t,true);break;
        case CrosshairStyle::Circle:DrawCircle(ctx,cx,cy,sz,c,t,false);break;
        case CrosshairStyle::TShape:DrawLine(ctx,cx-sz,cy-sz,cx+sz,cy-sz,c,t);DrawLine(ctx,cx,cy-sz,cx,cy+sz,c,t);break;
        case CrosshairStyle::Chevron:DrawLine(ctx,cx-sz,cy+sz,cx,cy-sz,c,t);DrawLine(ctx,cx,cy-sz,cx+sz,cy+sz,c,t);break;
        }
    }
    void CFeatureManager::RenderWatermark(ID3D11DeviceContext* ctx, int w, int h) {
        DrawText(ctx,"Rust External Premium",10.0f,10.0f,0x80FFFFFF,false,1.0f);
        if(m_uiCfg.showFPSCounter){char b[32];sprintf_s(b,"FPS: 144");DrawText(ctx,b,10.0f,24.0f,0x80FFFFFF,false,1.0f);}
    }

    bool CFeatureManager::ExportVisualPresets(const std::string& path) const {
        std::ofstream out(path, std::ios::out | std::ios::trunc);
        if (!out.is_open()) {
            return false;
        }

        out << "# Aether ESP preset export\n";
        out << "recent=";
        for (size_t i = 0; i < m_visualsCfg.recentESPColors.size(); ++i) {
            out << ColorToString(m_visualsCfg.recentESPColors[i]);
            if (i + 1 < m_visualsCfg.recentESPColors.size()) {
                out << ",";
            }
        }
        out << "\n";

        for (size_t i = 0; i < m_visualsCfg.customPresets.size(); ++i) {
            const auto& preset = m_visualsCfg.customPresets[i];
            out << "preset" << i << "=";
            if (!preset.occupied) {
                out << "0\n";
                continue;
            }

            std::string nameStr = preset.name.data();
            for (char& ch : nameStr) {
                if (ch == '|') ch = '-';
            }

            out << "1|" << nameStr << "|"
                << (preset.customESP ? "1" : "0") << "|" << ColorToString(preset.espColor) << "|"
                << (preset.customTeam ? "1" : "0") << "|" << ColorToString(preset.teammateColor) << "|"
                << ColorToString(preset.enemyColor) << "|" << ColorToString(preset.neutralColor) << "\n";
        }

        return true;
    }

    bool CFeatureManager::ImportVisualPresets(const std::string& path) {
        std::ifstream in(path);
        if (!in.is_open()) {
            return false;
        }

        auto recent = m_visualsCfg.recentESPColors;
        auto presets = m_visualsCfg.customPresets;

        std::string line;
        while (std::getline(in, line)) {
            line = TrimCopy(line);
            if (line.empty() || line[0] == '#') {
                continue;
            }

            if (line.rfind("recent=", 0) == 0) {
                std::string values = line.substr(7);
                std::stringstream ss(values);
                std::string token;
                size_t idx = 0;
                while (std::getline(ss, token, ',') && idx < recent.size()) {
                    uint32_t color;
                    if (ParseColor(token, color)) {
                        recent[idx] = color;
                    }
                    ++idx;
                }
                continue;
            }

            if (line.rfind("preset", 0) == 0) {
                size_t eq = line.find('=');
                if (eq == std::string::npos) {
                    continue;
                }
                std::string indexStr = line.substr(6, eq - 6);
                size_t presetIndex = 0;
                try {
                    presetIndex = static_cast<size_t>(std::stoul(indexStr));
                } catch (...) {
                    continue;
                }
                if (presetIndex >= presets.size()) {
                    continue;
                }

                std::string content = line.substr(eq + 1);
                content = TrimCopy(content);
                if (content == "0" || content.empty()) {
                    presets[presetIndex] = PlayerVisualsConfig::ESPColorPreset{};
                    continue;
                }

                std::array<std::string, 8> tokens{};
                size_t tokenIdx = 0;
                size_t start = 0;
                while (tokenIdx < tokens.size()) {
                    size_t pos = content.find('|', start);
                    if (pos == std::string::npos) {
                        tokens[tokenIdx++] = content.substr(start);
                        break;
                    } else {
                        tokens[tokenIdx++] = content.substr(start, pos - start);
                        start = pos + 1;
                    }
                }
                if (tokenIdx < 8) {
                    continue;
                }

                PlayerVisualsConfig::ESPColorPreset preset{};
                preset.occupied = TrimCopy(tokens[0]) == "1";
                std::string nameToken = tokens[1];
                std::snprintf(preset.name.data(), preset.name.size(), "%s", TrimCopy(nameToken).c_str());
                preset.customESP = TrimCopy(tokens[2]) == "1";
                if (!ParseColor(tokens[3], preset.espColor)) {
                    preset.espColor = m_visualsCfg.customESPColorValue;
                }
                preset.customTeam = TrimCopy(tokens[4]) == "1";
                if (!ParseColor(tokens[5], preset.teammateColor)) preset.teammateColor = m_visualsCfg.teammateCustomColor;
                if (!ParseColor(tokens[6], preset.enemyColor)) preset.enemyColor = m_visualsCfg.enemyCustomColor;
                if (!ParseColor(tokens[7], preset.neutralColor)) preset.neutralColor = m_visualsCfg.neutralCustomColor;

                presets[presetIndex] = preset;
            }
        }

        m_visualsCfg.recentESPColors = recent;
        m_visualsCfg.customPresets = presets;
        return true;
    }
    void CFeatureManager::ApplyWorldExploits() {
        if(!m_memory)return; uint64_t b=m_memory->GetBaseAddress(),ga=m_memory->GetGameAssemblyBase();
        if(m_exploitsCfg.fovChanger){uint64_t a=m_memory->Read<uint64_t>(ga+0x31F0058);if(a){a=m_memory->Read<uint64_t>(a+0xB8);if(a)m_memory->Write<float>(a+0x160,m_exploitsCfg.fovValue);}}
        if(m_exploitsCfg.brightNight){uint64_t a=m_memory->Read<uint64_t>(b+0x31F0058);if(a)m_memory->Write<float>(a+0x1C4,m_exploitsCfg.nightBrightness);}
        if(m_exploitsCfg.brightCave){uint64_t a=m_memory->Read<uint64_t>(b+0x31F0058);if(a)m_memory->Write<float>(a+0x1C8,m_exploitsCfg.caveBrightness);}
        if(m_exploitsCfg.brightStars){uint64_t a=m_memory->Read<uint64_t>(b+0x31F0058);if(a)m_memory->Write<float>(a+0x1CC,1.0f);}
        if(m_exploitsCfg.removeSun){uint64_t a=m_memory->Read<uint64_t>(b+0x31F0058);if(a)m_memory->Write<float>(a+0x1D0,0.0f);}
        if(m_exploitsCfg.removeMoon){uint64_t a=m_memory->Read<uint64_t>(b+0x31F0058);if(a)m_memory->Write<float>(a+0x1D4,0.0f);}
        if(m_exploitsCfg.removeLayers){uint64_t a=m_memory->Read<uint64_t>(b+0x31F0058);if(a)m_memory->Write<float>(a+0x1D8,0.0f);}
        if(m_exploitsCfg.removeScreenShake){uint64_t a=m_memory->Read<uint64_t>(ga+0x31F0058);if(a)m_memory->Write<float>(a+0x1DC,0.0f);}
        if(m_exploitsCfg.removeFlash){uint64_t a=m_memory->Read<uint64_t>(ga+0x31F0058);if(a)m_memory->Write<float>(a+0x1E0,0.0f);}
        if(m_exploitsCfg.instantRevive){uint64_t l=m_memory->GetLocalPlayer().address;if(l)m_memory->Write<float>(l+0x210,100.0f);}
        if(m_exploitsCfg.removeWearRestrictions){uint64_t a=m_memory->Read<uint64_t>(ga+0x31F0058);if(a)m_memory->Write<uint32_t>(a+0x1E4,0);}
        if(m_exploitsCfg.flyhack){bool ok=true;if(m_exploitsCfg.flyhackKey.key)ok=m_exploitsCfg.flyhackKey.mode==KeybindMode::Hold?(GetAsyncKeyState(m_exploitsCfg.flyhackKey.key)&0x8000):m_exploitsCfg.flyhackKey.enabled;if(ok){const auto&l=m_memory->GetLocalPlayer();if(l.address){Memory::Vector3 np=l.position;float s=m_exploitsCfg.flySpeed*0.01f;if(GetAsyncKeyState('W')&0x8000)np.z+=s;if(GetAsyncKeyState('S')&0x8000)np.z-=s;if(GetAsyncKeyState('A')&0x8000)np.x-=s;if(GetAsyncKeyState('D')&0x8000)np.x+=s;if(GetAsyncKeyState(VK_SPACE)&0x8000)np.y+=s;if(GetAsyncKeyState(VK_CONTROL)&0x8000)np.y-=s;m_memory->Write<Memory::Vector3>(l.address+0x90,np);}}}
        if(m_exploitsCfg.infiniteJump){uint64_t l=m_memory->GetLocalPlayer().address;if(l)m_memory->Write<float>(l+0x1E4,1.0f);}
        if(m_exploitsCfg.thirdPerson){uint64_t a=m_memory->Read<uint64_t>(ga+0x31F0058);if(a){a=m_memory->Read<uint64_t>(a+0xB8);if(a)m_memory->Write<float>(a+0x170,m_exploitsCfg.thirdPersonDistance);}}
        if(m_exploitsCfg.alwaysDay){uint64_t a=m_memory->Read<uint64_t>(b+0x31F0058);if(a)m_memory->Write<float>(a+0x1C0,12.0f);}
        if(m_exploitsCfg.fastLoot){uint64_t a=m_memory->Read<uint64_t>(ga+0x31F0058);if(a)m_memory->Write<float>(a+0x1E8,0.0f);}
        if(m_exploitsCfg.zoom){uint64_t a=m_memory->Read<uint64_t>(ga+0x31F0058);if(a)m_memory->Write<float>(a+0x174,m_exploitsCfg.zoomValue);}
    }
    void CFeatureManager::ApplyWeaponMods() {
        if(!m_memory)return; const auto& l=m_memory->GetLocalPlayer(); if(!l.address)return;
        if(m_weaponCfg.recoilModifier){uint64_t w=m_memory->Read<uint64_t>(l.address+0x5D0);if(w){m_memory->Write<float>(w+0x2C,m_weaponCfg.recoilX);m_memory->Write<float>(w+0x30,m_weaponCfg.recoilY);}}
        if(m_weaponCfg.weaponSpread){uint64_t w=m_memory->Read<uint64_t>(l.address+0x5D0);if(w)m_memory->Write<float>(w+0x34,m_weaponCfg.spreadValue);}
        if(m_weaponCfg.weaponSway){uint64_t w=m_memory->Read<uint64_t>(l.address+0x5D0);if(w)m_memory->Write<float>(w+0x38,m_weaponCfg.swayValue);}
        if(m_weaponCfg.rapidFire){uint64_t w=m_memory->Read<uint64_t>(l.address+0x5D0);if(w)m_memory->Write<float>(w+0x3C,0.001f);}
        if(m_weaponCfg.thickBullet){uint64_t w=m_memory->Read<uint64_t>(l.address+0x5D0);if(w)m_memory->Write<float>(w+0x40,2.0f);}
        if(m_weaponCfg.bigBullets){uint64_t w=m_memory->Read<uint64_t>(l.address+0x5D0);if(w)m_memory->Write<float>(w+0x44,3.0f);}
        if(m_weaponCfg.hitMaterialOverride){uint64_t w=m_memory->Read<uint64_t>(l.address+0x5D0);if(w)m_memory->Write<uint32_t>(w+0x48,m_weaponCfg.hitMaterial);}
        if(m_weaponCfg.hitboxOverride){uint64_t w=m_memory->Read<uint64_t>(l.address+0x5D0);if(w)m_memory->Write<uint32_t>(w+0x4C,m_weaponCfg.hitboxOverrideValue);}
    }
    void CFeatureManager::ApplyMovement() {
        if(!m_memory)return; const auto& l=m_memory->GetLocalPlayer(); if(!l.address)return;
        if(m_movementCfg.spiderman){bool ok=true;if(m_movementCfg.spidermanKey.key)ok=m_movementCfg.spidermanKey.mode==KeybindMode::Hold?(GetAsyncKeyState(m_movementCfg.spidermanKey.key)&0x8000):m_movementCfg.spidermanKey.enabled;if(ok){Memory::Vector3 p=l.position;float s=m_movementCfg.spiderManSpeed*0.01f;if(GetAsyncKeyState('W')&0x8000)p.z+=s;if(GetAsyncKeyState('S')&0x8000)p.z-=s;if(GetAsyncKeyState('A')&0x8000)p.x-=s;if(GetAsyncKeyState('D')&0x8000)p.x+=s;m_memory->Write<Memory::Vector3>(l.address+0x90,p);}}
        if(m_movementCfg.forceSprint||m_movementCfg.omnisprint)m_memory->Write<float>(l.address+0x1E8,1.0f);
        if(m_movementCfg.noFallDamage)m_memory->Write<float>(l.address+0x1EC,0.0f);
        if(m_movementCfg.highJump&&(GetAsyncKeyState(VK_SPACE)&0x8000)){Memory::Vector3 v=m_memory->Read<Memory::Vector3>(l.address+0x9C);v.y=m_movementCfg.jumpHeight;m_memory->Write<Memory::Vector3>(l.address+0x9C,v);}
        if(m_movementCfg.spinbot){bool ok=true;if(m_movementCfg.spinbotKey.key)ok=m_movementCfg.spinbotKey.mode==KeybindMode::Hold?(GetAsyncKeyState(m_movementCfg.spinbotKey.key)&0x8000):m_movementCfg.spinbotKey.enabled;if(ok){static float sa=0;sa+=m_movementCfg.spinSpeed;if(sa>360)sa-=360;Memory::Vector2 ang={sa,0};uint64_t a=m_memory->GetBaseAddress()+0x31F0058;if(a){a=m_memory->Read<uint64_t>(a);if(a){a=m_memory->Read<uint64_t>(a+0xB8);if(a){a=m_memory->Read<uint64_t>(a);if(a)m_memory->Write<Memory::Vector2>(a+0x2E4,ang);}}}}}
        if(m_movementCfg.instantSuicide){m_memory->Write<float>(l.address+0x200,0.0f);m_movementCfg.instantSuicide=false;}
    }
    uint32_t CFeatureManager::GetEntityColor(const Memory::GameEntity& e) const {
        auto pickColor = [&](uint32_t baseColor, uint32_t customColor) {
            return (m_visualsCfg.customTeamColors ? customColor : baseColor);
        };

        if(e.isTeammate && m_visualsCfg.colorizeTeams) {
            return pickColor(m_visualsCfg.teammateColor, m_visualsCfg.teammateCustomColor);
        }

        switch(e.type){
        case Memory::EntityType::Player:
            return pickColor(m_visualsCfg.enemyColor, m_visualsCfg.enemyCustomColor);
        case Memory::EntityType::Scientist:
            return pickColor(m_visualsCfg.neutralColor, m_visualsCfg.neutralCustomColor);
        default:
            return pickColor(m_visualsCfg.enemyColor, m_visualsCfg.enemyCustomColor);
        }
    }
    bool CFeatureManager::InitRenderResources(ID3D11Device* d) {
        m_d3dDevice=d;
        const char* shaderCode=R"(
            struct VSIn{float2 p:POSITION;float4 c:COLOR;};
            struct PSIn{float4 p:SV_POSITION;float4 c:COLOR;};
            PSIn VS(VSIn i){PSIn o;o.p=float4(i.p,0,1);o.c=i.c;return o;}
            float4 PS(PSIn i):SV_TARGET{return i.c;}
        )";
        ID3DBlob *vsb=nullptr,*psb=nullptr,*eb=nullptr;
        D3DCompile(shaderCode,strlen(shaderCode),"shader",nullptr,nullptr,"VS","vs_4_0",0,0,&vsb,&eb);
        D3DCompile(shaderCode,strlen(shaderCode),"shader",nullptr,nullptr,"PS","ps_4_0",0,0,&psb,&eb);
        if(!vsb||!psb){if(vsb)vsb->Release();if(psb)psb->Release();return false;}
        d->CreateVertexShader(vsb->GetBufferPointer(),vsb->GetBufferSize(),nullptr,&m_vs);
        d->CreatePixelShader(psb->GetBufferPointer(),psb->GetBufferSize(),nullptr,&m_ps);
        D3D11_INPUT_ELEMENT_DESC ied[]={{"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},{"COLOR",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,8,D3D11_INPUT_PER_VERTEX_DATA,0}};
        d->CreateInputLayout(ied,2,vsb->GetBufferPointer(),vsb->GetBufferSize(),&m_inputLayout);
        vsb->Release();psb->Release();
        D3D11_BLEND_DESC bld={};bld.RenderTarget[0].BlendEnable=TRUE;
        bld.RenderTarget[0].SrcBlend=D3D11_BLEND_SRC_ALPHA;bld.RenderTarget[0].DestBlend=D3D11_BLEND_INV_SRC_ALPHA;
        bld.RenderTarget[0].BlendOp=D3D11_BLEND_OP_ADD;bld.RenderTarget[0].SrcBlendAlpha=D3D11_BLEND_ONE;
        bld.RenderTarget[0].DestBlendAlpha=D3D11_BLEND_ZERO;bld.RenderTarget[0].BlendOpAlpha=D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].RenderTargetWriteMask=D3D11_COLOR_WRITE_ENABLE_ALL;
        d->CreateBlendState(&bld,&m_blendState);
        m_renderInit=true;return true;
    }

    void CFeatureManager::BeginBatch(ID3D11DeviceContext* ctx) {
        m_vertices.clear();m_indices.clear();
    }

    void CFeatureManager::FlushBatch(ID3D11DeviceContext* ctx) {
        if(m_vertices.empty()||!m_renderInit)return;
        if(m_vb){m_vb->Release();m_vb=nullptr;}
        if(m_ib){m_ib->Release();m_ib=nullptr;}
        D3D11_BUFFER_DESC bd={};bd.Usage=D3D11_USAGE_DEFAULT;bd.BindFlags=D3D11_BIND_VERTEX_BUFFER;
        bd.ByteWidth=(UINT)(m_vertices.size()*sizeof(Vertex));
        D3D11_SUBRESOURCE_DATA sd={m_vertices.data()};
        m_d3dDevice->CreateBuffer(&bd,&sd,&m_vb);
        bd.BindFlags=D3D11_BIND_INDEX_BUFFER;
        bd.ByteWidth=(UINT)(m_indices.size()*sizeof(uint32_t));
        sd.pSysMem=m_indices.data();
        m_d3dDevice->CreateBuffer(&bd,&sd,&m_ib);
        UINT stride=sizeof(Vertex),offset=0;
        ctx->IASetVertexBuffers(0,1,&m_vb,&stride,&offset);
        ctx->IASetIndexBuffer(m_ib,DXGI_FORMAT_R32_UINT,0);
        ctx->IASetInputLayout(m_inputLayout);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->VSSetShader(m_vs,nullptr,0);
        ctx->PSSetShader(m_ps,nullptr,0);
        float bf[4]={0};ctx->OMSetBlendState(m_blendState,bf,0xFFFFFFFF);
        ctx->DrawIndexed((UINT)m_indices.size(),0,0);
        m_vertices.clear();m_indices.clear();
    }

    void CFeatureManager::DrawBox(ID3D11DeviceContext* ctx, float x, float y, float w, float h, uint32_t c, float t) {
        DrawFilledRect(ctx,x,y,w,t,c);
        DrawFilledRect(ctx,x,y+h-t,w,t,c);
        DrawFilledRect(ctx,x,y,t,h,c);
        DrawFilledRect(ctx,x+w-t,y,t,h,c);
    }

    void CFeatureManager::DrawCornerBox(ID3D11DeviceContext* ctx, float x, float y, float w, float h, uint32_t c, float t, float cl) {
        float cl2=std::min(cl,std::min(w,h)*0.5f);
        DrawFilledRect(ctx,x,y,cl2,t,c);DrawFilledRect(ctx,x,y,t,cl2,c);
        DrawFilledRect(ctx,x+w-cl2,y,cl2,t,c);DrawFilledRect(ctx,x+w-t,y,t,cl2,c);
        DrawFilledRect(ctx,x+w-cl2,y+h-t,cl2,t,c);DrawFilledRect(ctx,x+w-t,y+h-cl2,t,cl2,c);
        DrawFilledRect(ctx,x,y+h-t,cl2,t,c);DrawFilledRect(ctx,x,y+h-cl2,t,cl2,c);
    }

    void CFeatureManager::DrawLine(ID3D11DeviceContext* ctx, float x1, float y1, float x2, float y2, uint32_t c, float t) {
        float dx=x2-x1,dy=y2-y1,len=sqrtf(dx*dx+dy*dy);
        if(len<0.001f)return;
        float nx=-dy/len*t*0.5f,ny=dx/len*t*0.5f;
        uint32_t base=(uint32_t)m_vertices.size();
        m_vertices.push_back({x1+nx,y1+ny,0,1,c});
        m_vertices.push_back({x1-nx,y1-ny,0,1,c});
        m_vertices.push_back({x2-nx,y2-ny,0,1,c});
        m_vertices.push_back({x2+nx,y2+ny,0,1,c});
        m_indices.insert(m_indices.end(),{base,base+1,base+2,base,base+2,base+3});
    }

    void CFeatureManager::DrawFilledRect(ID3D11DeviceContext* ctx, float x, float y, float w, float h, uint32_t c) {
        uint32_t base=(uint32_t)m_vertices.size();
        m_vertices.push_back({x,y,0,1,c});
        m_vertices.push_back({x+w,y,0,1,c});
        m_vertices.push_back({x+w,y+h,0,1,c});
        m_vertices.push_back({x,y+h,0,1,c});
        m_indices.insert(m_indices.end(),{base,base+1,base+2,base,base+2,base+3});
    }

    void CFeatureManager::DrawCircle(ID3D11DeviceContext* ctx, float cx, float cy, float r, uint32_t c, float t, bool filled) {
        int segs=64;float step=6.2831853f/segs;
        if(filled){
            uint32_t base=(uint32_t)m_vertices.size();
            m_vertices.push_back({cx,cy,0,1,c});
            for(int i=0;i<=segs;i++){float a=i*step;m_vertices.push_back({cx+cosf(a)*r,cy+sinf(a)*r,0,1,c});}
            for(int i=0;i<segs;i++)m_indices.insert(m_indices.end(),{base,(uint32_t)(base+1+i),(uint32_t)(base+2+i)});
        }else{
            for(int i=0;i<segs;i++){
                float a1=i*step,a2=(i+1)*step;
                float x1=cx+cosf(a1)*r,y1=cy+sinf(a1)*r,x2=cx+cosf(a2)*r,y2=cy+sinf(a2)*r;
                DrawLine(ctx,x1,y1,x2,y2,c,t);
            }
        }
    }

    void CFeatureManager::DrawText(ID3D11DeviceContext*, const std::string&, float, float, uint32_t, bool, float) {}

    void CFeatureManager::DrawTriangle(ID3D11DeviceContext* ctx, float x1, float y1, float x2, float y2, float x3, float y3, uint32_t c, bool filled) {
        if(filled){
            uint32_t base=(uint32_t)m_vertices.size();
            m_vertices.push_back({x1,y1,0,1,c});m_vertices.push_back({x2,y2,0,1,c});m_vertices.push_back({x3,y3,0,1,c});
            m_indices.insert(m_indices.end(),{base,base+1,base+2});
        }else{
            DrawLine(ctx,x1,y1,x2,y2,c,1.0f);DrawLine(ctx,x2,y2,x3,y3,c,1.0f);DrawLine(ctx,x3,y3,x1,y1,c,1.0f);
        }
    }

}
