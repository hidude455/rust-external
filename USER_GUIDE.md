# User Guide - Rust Anti-Cheat Evasion System

## Quick Start Guide

### Step 1: Build the Software
1. Download/clone the project to your computer
2. Right-click `build_final.bat` and select "Run as administrator"
3. Wait for the build to complete (this may take several minutes)
4. The executable will be generated in the output directory

### Step 2: Launch the Software
1. Start the Rust game first (let it fully load to the main menu)
2. Run the generated executable as administrator
3. The software will automatically attach to the Rust process
4. Press **INSERT** to open the menu

### Step 3: Configure Your Settings
- Use the in-game menu to configure ESP, aimbot, and other features
- Adjust settings to your preference
- Save your configuration profile

---

## Controls & Hotkeys

### Menu Controls
- **INSERT**: Open/close the cheat menu
- **F1**: Toggle ESP on/off
- **F2**: Toggle recoil compensation
- **F3**: Toggle aimbot
- **F4**: Toggle projectile prediction
- **F5**: Toggle resource automation
- **F6**: Toggle auto-healing
- **F7**: Toggle combat assistance
- **F8**: Toggle stealth mode
- **F9**: Emergency shutdown (close everything safely)
- **F10**: Save current configuration

### In-Game Menu Navigation
- **Mouse**: Click to select options and adjust sliders
- **Arrow Keys**: Navigate menu items
- **Enter**: Confirm selection
- **Escape**: Close sub-menus

---

## Feature Usage

### ESP System (Wallhack)
**What it does**: Shows you information about other players, resources, and items through walls.

**How to use**:
1. Press **F1** to toggle ESP on/off
2. Open menu with **INSERT**
3. Navigate to ESP settings
4. Choose what you want to see:
   - Players (names, health, distance, weapons)
   - Resources (wood, stone, metal, sulfur, etc.)
   - Containers (boxes, bodies, safes)
   - Vehicles (boats, cars, helicopters)
5. Adjust distance limits to improve performance
6. Customize colors for each entity type

**ESP Modes**:
- **Basic**: Simple boxes and text
- **Advanced**: Detailed information with health bars
- **Skeleton**: Shows player skeleton/bones
- **Chams**: Colors players through walls
- **Glow**: Glowing effect on entities
- **Radar**: Mini-map showing nearby entities

---

### Aimbot System
**What it does**: Automatically aims at enemies for you.

**How to use**:
1. Press **F3** to toggle aimbot on/off
2. Open menu with **INSERT**
3. Navigate to Aimbot settings
4. Configure your preferred settings:
   - **Aim Mode**: Choose between legit, rage, silent, etc.
   - **FOV**: Field of view (how wide the aimbot looks)
   - **Smoothness**: How natural the movement looks (higher = more human)
   - **Target Priority**: Distance, health, or crosshair proximity
5. Set your aim key (default: right mouse button)
6. In-game, hold your aim key when near enemies

**Aim Modes**:
- **Legit**: Subtle, human-like aiming
- **Rage**: Aggressive, instant locking
- **Silent**: Aims without moving your crosshair visibly
- **Perfect Silent**: Completely invisible aiming
- **Smooth**: Smooth, natural movement
- **Predictive**: Accounts for enemy movement

**Safety Tips**:
- Keep smoothness above 5.0 for legit play
- Use smaller FOV (2-5) to avoid suspicion
- Enable visibility checks to only aim at visible enemies
- Use team filtering to avoid targeting teammates

---

### Resource Automation
**What it does**: Automatically gathers and crafts resources for you.

**How to use**:
1. Press **F5** to toggle resource automation on/off
2. Open menu with **INSERT**
3. Navigate to Resource settings
4. Select which resources to auto-gather:
   - Wood, stone, metal ore
   - Sulfur, high quality metal
   - Cloth, food, medical items
5. Set gathering priorities
6. Configure crafting automation:
   - Select items to auto-craft
   - Set crafting priorities
7. Enable auto-deposit to base/TC

**Smart Features**:
- Avoids other players automatically
- Steers clear of radiation zones
- Optimizes gathering paths
- Deposits resources when inventory is full

---

### Combat Assistance
**What it does**: Helps you in combat situations automatically.

**How to use**:
1. Press **F7** to toggle combat assistance on/off
2. Open menu with **INSERT**
3. Navigate to Combat settings
4. Enable desired features:
   - Auto-healing (uses medkits when low)
   - Auto-reload (reloads when empty)
   - Auto-dodge (strafes when shot)
   - Jump assistance (times jumps perfectly)
5. Configure health thresholds
6. Set weapon preferences

---

### Stealth & Anti-Detection
**What it does**: Helps you avoid being detected by anti-cheat systems.

**How to use**:
1. Press **F8** to toggle stealth mode on/off
2. Open menu with **INSERT**
3. Navigate to Stealth settings
4. Adjust stealth level (0-10):
   - 0-2: Basic protection (minimal performance impact)
   - 3-5: Standard protection (balanced)
   - 6-8: High protection (some performance impact)
   - 9-10: Maximum protection (significant performance impact)
5. Enable additional protection:
   - Memory scrambling
   - Process hiding
   - Network obfuscation
   - Hardware spoofing

**Recommendations**:
- Use stealth level 3-5 for normal play
- Increase to 6-8 if you suspect detection
- Only use 9-10 for critical situations
- Monitor detection events in logs

---

## Configuration Profiles

### Saving Profiles
1. Configure all your settings
2. Press **F10** or use menu option
3. Name your profile
4. Profile is saved to disk

### Loading Profiles
1. Open menu with **INSERT**
2. Navigate to Profiles section
3. Select your saved profile
4. Click "Load"

### Recommended Profiles
- **Legit Play**: Low smoothness, small FOV, basic ESP
- **Rage Play**: High smoothness, large FOV, all features
- **Farm Mode**: Resource automation, minimal combat
- **Stealth Mode**: Maximum stealth, minimal features

---

## Performance Optimization

### If You Experience Lag:
1. Reduce ESP distance limits
2. Disable advanced ESP modes (skeleton, chams)
3. Lower stealth level
4. Disable unnecessary features
5. Close background applications
6. Check FPS counter in menu (target: 60+ FPS)

### If Game Crashes:
1. Run software as administrator
2. Disable kernel-level features
3. Lower stealth level
4. Check logs in `Logs/` folder
5. Ensure Rust is updated

### If Features Don't Work:
1. Verify Rust is running before launching
2. Check that software attached successfully (see logs)
3. Try restarting both Rust and the software
4. Update to latest version

---

## Safety Guidelines

### To Reduce Detection Risk:
1. **Don't be obvious**: Use legit settings, don't snap to targets
2. **Vary your playstyle**: Don't use the same settings every time
3. **Take breaks**: Don't use for extended periods
4. **Avoid reports**: Don't make other players suspicious
5. **Update regularly**: Keep software updated for latest bypasses
6. **Use stealth**: Enable stealth mode, especially on official servers

### What NOT to Do:
- Don't rage in chat or be toxic
- Don't stream or record while using
- Don't share your configuration publicly
- Don't use on high-profile accounts
- Don't ignore detection warnings in logs

---

## Troubleshooting

### "Process Attachment Failed"
- Make sure Rust is running
- Run as administrator
- Check Rust process name in logs
- Restart both Rust and the software

### "DirectX Initialization Failed"
- Update graphics drivers
- Ensure DirectX 11 is installed
- Run in windowed mode
- Disable fullscreen optimizations

### "Menu Won't Open"
- Check if INSERT key is bound to something else
- Try alternative hotkeys in settings
- Verify software is running (check task manager)
- Restart the software

### "ESP Not Showing"
- Verify ESP is enabled (F1)
- Check distance settings (might be too low)
- Ensure entities are in range
- Try different ESP modes
- Check if stealth mode is interfering

### "Aimbot Not Working"
- Verify aimbot is enabled (F3)
- Check aim key binding
- Ensure enemies are in FOV range
- Check if team filtering is blocking targets
- Verify visibility checks aren't too strict

---

## Understanding the Logs

Logs are saved in the `Logs/` folder with timestamped filenames.

### What to Look For:
- **SUCCESS**: Software attached correctly
- **ERROR**: Something went wrong (check troubleshooting)
- **WARNING**: Potential issues or detection events
- **DETECTION**: Anti-cheat detected something (take action!)

### Log Analysis:
- Regularly check logs for detection events
- Look for patterns in errors
- Export logs if you need support

---

## Emergency Procedures

### If You Suspect Detection:
1. Press **F9** for emergency shutdown
2. Close Rust immediately
3. Do not reconnect to the server
4. Check logs for confirmation
5. Wait 24-48 hours before using again

### If You Get Kicked:
1. Note the kick reason
2. Check logs for related events
3. Adjust settings based on kick reason
4. Consider using different server

### If You Get Banned:
- Account bans are usually permanent
- Hardware bans may affect your PC
- Consider hardware spoofing features
- Create new account if necessary

---

## Advanced Tips

### Custom Configurations:
- Experiment with different settings
- Create profiles for different situations
- Fine-tune smoothness and FOV for your playstyle
- Adjust colors for better visibility

### Staying Undetected:
- Use human-like settings
- Randomize your configuration occasionally
- Monitor your stats and performance
- Stay updated on anti-cheat changes

### Maximizing Performance:
- Only enable features you actually use
- Adjust render distance based on your PC
- Use performance mode in Rust settings
- Close unnecessary background apps

---

## FAQ

**Q: Will I get banned?**
A: There's always a risk. Use stealth mode, legit settings, and follow safety guidelines to minimize risk.

**Q: Can I use this on official servers?**
A: Not recommended. Official servers have stronger anti-cheat. Use on community servers with weaker protection.

**Q: How often should I update?**
A: Update whenever a new version is released, especially after Rust game updates.

**Q: Can I use this alongside other cheats?**
A: Not recommended. Multiple cheats increase detection risk significantly.

**Q: What if the software stops working after a Rust update?**
A: Wait for a software update that addresses the new Rust version. Don't use outdated versions.

**Q: Is this safe for my main account?**
A: No. Always use on a secondary/throwaway account. There's always a ban risk.

---

## Legal Disclaimer

This software is for educational purposes only. Using cheats in online games violates terms of service and may result in permanent account bans. The creators are not responsible for any consequences resulting from the use of this software. Use at your own risk and comply with all applicable laws and regulations.

---

## Need Help?

1. Check this guide first
2. Review the logs in the `Logs/` folder
3. Try the troubleshooting section
4. Check for software updates
5. Contact support through official channels (if available)

---

**Version**: 2.0.0
**Last Updated**: 2026
**Status**: User Guide Complete
