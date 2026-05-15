# 🎮 UNITY OBJECT SETUP GUIDE

## 🚀 **STEP-BY-STEP SETUP**

### **1. PLAYER OBJECT SETUP**

#### **Create Player GameObject:**
1. **Right-click** in Hierarchy window
2. **Create Empty** → Name it "Player"
3. **Add Components:**
   - Click "Add Component" → Search "Character Controller"
   - Click "Add Component" → Search "PlayerController" (your script)
   - Click "Add Component" → Search "PlayerHealth" (your script)
   - Click "Add Component" → Search "WeaponSystem" (your script)

#### **Setup Player Camera:**
1. **Right-click** on Player → Create Empty → Name "CameraOffset"
2. **Right-click** on CameraOffset → Camera → Name "MainCamera"
3. **Position CameraOffset:** (0, 1.6, 0)
4. **Position MainCamera:** (0, 0, 0.5)
5. **In PlayerController script:** Drag MainCamera to "playerCamera" field

#### **Setup Player Collider:**
1. **Select Player** object
2. **In Character Controller:** Set Height = 2, Radius = 0.5
3. **Center:** (0, 1, 0)

#### **Tag Player:**
1. **Select Player** object
2. **In Inspector:** Click "Tag" dropdown → Select "Player" (or create it)

---

### **2. ENEMY OBJECT SETUP**

#### **Create Enemy GameObject:**
1. **Right-click** in Hierarchy → Create Empty → Name "Enemy"
2. **Add Components:**
   - Click "Add Component" → Search "NavMeshAgent"
   - Click "Add Component" → Search "EnemyAI" (your script)
   - Click "Add Component" → Search "EnemyHealth" (your script)
   - Click "Add Component" → Search "Capsule Collider"

#### **Setup Enemy Collider:**
1. **Select Enemy** object
2. **In Capsule Collider:** Set Height = 2, Radius = 0.5
3. **Center:** (0, 1, 0)

#### **Setup Enemy AI:**
1. **Select Enemy** object
2. **In EnemyAI script:**
   - Set "moveSpeed" = 3
   - Set "detectionRange" = 20
   - Set "attackRange" = 5
   - Drag Player object to "player" field

#### **Setup NavMeshAgent:**
1. **Select Enemy** object
2. **In NavMeshAgent:**
   - Set "Speed" = 3
   - Set "Angular Speed" = 120
   - Set "Acceleration" = 8
   - Set "Stopping Distance" = 0.1

#### **Create Enemy Health Bar:**
1. **Right-click** on Enemy → UI → Canvas → Name "HealthBarCanvas"
2. **Right-click** on HealthBarCanvas → UI → Slider → Name "HealthBar"
3. **Setup Canvas:**
   - Render Mode: "World Space"
   - Position: (0, 2, 0)
   - Rotation: (0, 0, 0)
   - Scale: (0.01, 0.01, 0.01)
4. **Setup Slider:**
   - Anchor: "middle-center"
   - Position: (0, 0, 0)
   - Width: 100, Height: 10
5. **In EnemyHealth script:** Drag HealthBar to "healthBar" field

#### **Tag Enemy:**
1. **Select Enemy** object
2. **In Inspector:** Click "Tag" dropdown → Select "Enemy"

---

### **3. WEAPON OBJECT SETUP**

#### **Create Weapon GameObject:**
1. **Right-click** on Player → Create Empty → Name "Weapon"
2. **Position:** (0.5, -0.3, 0.5)
3. **Add Components:**
   - Click "Add Component" → Search "WeaponSystem" (your script)

#### **Setup Weapon System:**
1. **Select Weapon** object
2. **In WeaponSystem script:**
   - Set "damage" = 25
   - Set "range" = 100
   - Set "fireRate" = 600
   - Set "magazineSize" = 30
   - Drag MainCamera to "playerCamera" field

#### **Create Visual Weapon (Optional):**
1. **Right-click** on Weapon → 3D Object → Cube → Name "GunModel"
2. **Scale:** (0.1, 0.1, 0.5)
3. **Position:** (0, 0, 0.25)

---

## 🏗️ **LEVEL SETUP**

### **1. Create Ground:**
1. **Right-click** in Hierarchy → 3D Object → Plane
2. **Position:** (0, 0, 0)
3. **Scale:** (10, 1, 10)

### **2. Create Walls:**
1. **Right-click** → 3D Object → Cube → Name "Wall1"
2. **Position:** (5, 1, 0)
3. **Scale:** (0.5, 2, 10)
4. **Duplicate** for other walls

### **3. Add Lighting:**
1. **Right-click** → Light → Directional Light
2. **Rotation:** (50, -30, 0)
3. **Intensity:** 1.0

---

## 🎯 **TESTING SETUP**

### **1. Create Multiple Enemies:**
1. **Select Enemy** object
2. **Ctrl+D** to duplicate
3. **Position** enemies around the level
4. **Rename** them "Enemy1", "Enemy2", etc.

### **2. Setup Enemy References:**
1. **Select each Enemy**
2. **In EnemyAI script:** Drag Player to "player" field
3. **In EnemyHealth script:** Drag HealthBar to "healthBar" field

### **3. Add ESP Testing Components:**
1. **Select each Enemy**
2. **Add Component:** Search "ESPTestTarget"
3. **Add Component:** Search "AimbotTestTarget"

---

## 🔧 **ADVANCED SETUP**

### **1. Create Spawn Points:**
1. **Right-click** → Create Empty → Name "SpawnPoint1"
2. **Position:** around the level
3. **Create multiple spawn points**

### **2. Create Cover Objects:**
1. **Right-click** → 3D Object → Cube → Name "Cover1"
2. **Scale:** (1, 2, 0.2)
3. **Position:** strategically around level

### **3. Setup Layer System:**
1. **Edit → Project Settings → Tags and Layers**
2. **Add layers:** "Player", "Enemy", "Environment"
3. **Set object layers accordingly**

---

## 🎮 **FINAL TESTING**

### **1. Test Basic Movement:**
1. **Press Play**
2. **Use WASD** to move
3. **Use Mouse** to look around
4. **Press Space** to jump

### **2. Test Shooting:**
1. **Left Click** to shoot
2. **Check if enemies take damage**
3. **Test reload (R key)**

### **3. Test Enemy AI:**
1. **Walk near enemies**
2. **Check if they chase you**
3. **Check if they attack when close**

### **4. Test Health System:**
1. **Check health bars** above enemies
2. **Verify player health** regeneration
3. **Test death mechanics**

---

## 🚨 **TROUBLESHOOTING**

### **Common Issues:**
- **Player not moving:** Check CharacterController setup
- **Camera not following:** Check camera positioning
- **Enemies not moving:** Check NavMeshAgent and bake NavMesh
- **Shooting not working:** Check WeaponSystem setup
- **Health bars not showing:** Check Canvas settings

### **NavMesh Setup:**
1. **Select ground plane**
2. **Check "Static" in Inspector**
3. **Window → AI → Navigation**
4. **Bake** the NavMesh

---

## 🎯 **READY TO TEST**

Once you complete this setup, you'll have:
- ✅ **Fully functional player** with movement and shooting
- ✅ **Smart enemy AI** that chases and attacks
- ✅ **Health system** with visual feedback
- ✅ **Perfect testing environment** for your anti-cheat system
- ✅ **ESP and aimbot testing targets** integrated

**Your Unity test game is now ready for anti-cheat testing!** 🎮
