# 🎮 UNITY TEST PROJECT TEMPLATE

## 🚀 **QUICK START GUIDE**

### **Step 1: Install Unity**
1. Download [Unity Hub](https://unity.com/download)
2. Install Unity 2022.3 LTS
3. Create new 3D project named "AntiCheatTestGame"

### **Step 2: Create Basic Scripts**
Create these scripts in your `Assets/Scripts/` folder:

---

## 📁 **ESSENTIAL SCRIPTS**

### **PlayerController.cs**
```csharp
using UnityEngine;

public class PlayerController : MonoBehaviour {
    [Header("Movement")]
    public float moveSpeed = 5f;
    public float mouseSensitivity = 2f;
    public float jumpForce = 5f;
    
    [Header("Camera")]
    public Camera playerCamera;
    public float cameraDistance = 0.5f;
    
    [Header("Weapon")]
    public WeaponSystem weapon;
    
    private CharacterController controller;
    private float verticalRotation = 0f;
    
    void Start() {
        controller = GetComponent<CharacterController>();
        
        // Lock cursor
        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;
        
        // Setup camera
        if (playerCamera == null) {
            playerCamera = Camera.main;
        }
    }
    
    void Update() {
        HandleMovement();
        HandleMouseLook();
        HandleShooting();
        HandleJumping();
        
        // Update camera position
        UpdateCameraPosition();
    }
    
    void HandleMovement() {
        float moveX = Input.GetAxis("Horizontal");
        float moveZ = Input.GetAxis("Vertical");
        
        Vector3 moveDirection = transform.right * moveX + transform.forward * moveZ;
        controller.Move(moveDirection * moveSpeed * Time.deltaTime);
    }
    
    void HandleMouseLook() {
        float mouseX = Input.GetAxis("Mouse X") * mouseSensitivity;
        float mouseY = Input.GetAxis("Mouse Y") * mouseSensitivity;
        
        // Rotate player around Y axis
        transform.Rotate(0f, mouseX, 0f);
        
        // Rotate camera around X axis
        verticalRotation -= mouseY;
        verticalRotation = Mathf.Clamp(verticalRotation, -90f, 90f);
        
        if (playerCamera != null) {
            playerCamera.transform.localRotation = Quaternion.Euler(verticalRotation, 0f, 0f);
        }
    }
    
    void HandleJumping() {
        if (controller.isGrounded && Input.GetButtonDown("Jump")) {
            Vector3 jumpVelocity = Vector3.up * jumpForce;
            controller.Move(jumpVelocity * Time.deltaTime);
        }
    }
    
    void HandleShooting() {
        if (Input.GetButtonDown("Fire1") && weapon != null) {
            weapon.Shoot();
        }
    }
    
    void UpdateCameraPosition() {
        if (playerCamera != null) {
            playerCamera.transform.position = transform.position - transform.forward * cameraDistance;
        }
    }
    
    // Public methods for anti-cheat testing
    public Vector3 GetPosition() => transform.position;
    public Quaternion GetRotation() => transform.rotation;
    public float GetHealth() => GetComponent<PlayerHealth>()?.GetCurrentHealth() ?? 0f;
    public bool IsGrounded() => controller.isGrounded;
}
```

### **EnemyAI.cs**
```csharp
using UnityEngine;
using UnityEngine.AI;

public class EnemyAI : MonoBehaviour {
    [Header("Enemy Stats")]
    public float maxHealth = 100f;
    public float currentHealth;
    public float moveSpeed = 3f;
    public float detectionRange = 20f;
    public float attackRange = 5f;
    
    [Header("AI Behavior")]
    public Transform player;
    public LayerMask playerLayer;
    
    [Header("ESP Testing")]
    public string enemyName = "TestEnemy";
    public bool isVisibleToPlayer = false;
    
    private NavMeshAgent agent;
    private EnemyHealth healthComponent;
    
    void Start() {
        currentHealth = maxHealth;
        agent = GetComponent<NavMeshAgent>();
        healthComponent = GetComponent<EnemyHealth>();
        
        if (player == null) {
            player = GameObject.FindGameObjectWithTag("Player")?.transform;
        }
        
        agent.speed = moveSpeed;
    }
    
    void Update() {
        if (player == null) return;
        
        float distanceToPlayer = Vector3.Distance(transform.position, player.position);
        
        // Check visibility
        isVisibleToPlayer = IsVisibleToPlayer();
        
        if (distanceToPlayer <= detectionRange) {
            // Chase player
            agent.SetDestination(player.position);
            
            if (distanceToPlayer <= attackRange) {
                // Attack player
                AttackPlayer();
            }
        } else {
            // Patrol or idle
            agent.SetDestination(transform.position);
        }
    }
    
    bool IsVisibleToPlayer() {
        RaycastHit hit;
        Vector3 directionToPlayer = (player.position - transform.position).normalized;
        
        if (Physics.Raycast(transform.position, directionToPlayer, out hit, detectionRange, playerLayer)) {
            return hit.collider.CompareTag("Player");
        }
        
        return false;
    }
    
    void AttackPlayer() {
        // Simple attack logic
        PlayerHealth playerHealth = player.GetComponent<PlayerHealth>();
        if (playerHealth != null) {
            playerHealth.TakeDamage(10f);
        }
    }
    
    public void TakeDamage(float damage) {
        currentHealth -= damage;
        
        if (healthComponent != null) {
            healthComponent.TakeDamage(damage);
        }
        
        if (currentHealth <= 0) {
            Die();
        }
    }
    
    void Die() {
        // Death logic
        Destroy(gameObject);
    }
    
    // Public methods for anti-cheat testing
    public Vector3 GetPosition() => transform.position;
    public Vector3 GetHeadPosition() => transform.position + Vector3.up * 1.6f;
    public float GetCurrentHealth() => currentHealth;
    public float GetMaxHealth() => maxHealth;
    public bool IsVisible() => isVisibleToPlayer;
    public string GetEnemyName() => enemyName;
    public bool IsAlive() => currentHealth > 0f;
}
```

### **WeaponSystem.cs**
```csharp
using UnityEngine;

public class WeaponSystem : MonoBehaviour {
    [Header("Weapon Stats")]
    public float damage = 25f;
    public float range = 100f;
    public float fireRate = 600f; // Rounds per minute
    public int magazineSize = 30;
    public int currentAmmo;
    public float reloadTime = 2f;
    
    [Header("Visual Effects")]
    public GameObject muzzleFlash;
    public GameObject impactEffect;
    public Camera playerCamera;
    
    private float nextFireTime;
    private bool isReloading = false;
    
    void Start() {
        currentAmmo = magazineSize;
        
        if (playerCamera == null) {
            playerCamera = Camera.main;
        }
    }
    
    void Update() {
        if (Input.GetButtonDown("Reload") && !isReloading) {
            StartCoroutine(Reload());
        }
    }
    
    public void Shoot() {
        if (isReloading || currentAmmo <= 0) return;
        
        if (Time.time >= nextFireTime) {
            nextFireTime = Time.time + (60f / fireRate);
            
            // Perform raycast
            RaycastHit hit;
            if (Physics.Raycast(playerCamera.transform.position, 
                              playerCamera.transform.forward, 
                              out hit, range)) {
                
                // Handle hit
                HandleHit(hit);
                
                // Create impact effect
                if (impactEffect != null) {
                    Instantiate(impactEffect, hit.point, Quaternion.LookRotation(hit.normal));
                }
            }
            
            // Show muzzle flash
            if (muzzleFlash != null) {
                muzzleFlash.SetActive(true);
                Invoke(nameof(HideMuzzleFlash), 0.1f);
            }
            
            currentAmmo--;
            
            // Auto reload if empty
            if (currentAmmo <= 0) {
                StartCoroutine(Reload());
            }
        }
    }
    
    void HandleHit(RaycastHit hit) {
        // Check if hit enemy
        EnemyAI enemy = hit.collider.GetComponent<EnemyAI>();
        if (enemy != null) {
            enemy.TakeDamage(damage);
        }
        
        // Check if hit player
        PlayerHealth player = hit.collider.GetComponent<PlayerHealth>();
        if (player != null) {
            player.TakeDamage(damage);
        }
    }
    
    void HideMuzzleFlash() {
        if (muzzleFlash != null) {
            muzzleFlash.SetActive(false);
        }
    }
    
    System.Collections.IEnumerator Reload() {
        isReloading = true;
        yield return new WaitForSeconds(reloadTime);
        currentAmmo = magazineSize;
        isReloading = false;
    }
    
    // Public methods for anti-cheat testing
    public float GetDamage() => damage;
    public float GetRange() => range;
    public int GetCurrentAmmo() => currentAmmo;
    public int GetMagazineSize() => magazineSize;
    public bool IsReloading() => isReloading;
    public Vector3 GetMuzzlePosition() => transform.position;
    public Vector3 GetAimDirection() => playerCamera.transform.forward;
}
```

### **PlayerHealth.cs**
```csharp
using UnityEngine;

public class PlayerHealth : MonoBehaviour {
    [Header("Health Settings")]
    public float maxHealth = 100f;
    public float currentHealth;
    public float regenerationRate = 1f;
    
    [Header("UI")]
    public UnityEngine.UI.Slider healthBar;
    public UnityEngine.UI.Text healthText;
    
    void Start() {
        currentHealth = maxHealth;
        UpdateUI();
    }
    
    void Update() {
        // Regeneration
        if (currentHealth < maxHealth) {
            currentHealth += regenerationRate * Time.deltaTime;
            currentHealth = Mathf.Min(currentHealth, maxHealth);
            UpdateUI();
        }
    }
    
    public void TakeDamage(float damage) {
        currentHealth -= damage;
        currentHealth = Mathf.Max(currentHealth, 0f);
        UpdateUI();
        
        if (currentHealth <= 0) {
            Die();
        }
    }
    
    public void Heal(float amount) {
        currentHealth += amount;
        currentHealth = Mathf.Min(currentHealth, maxHealth);
        UpdateUI();
    }
    
    void Die() {
        // Death logic
        Debug.Log("Player died!");
        // You could restart the level or show game over screen
    }
    
    void UpdateUI() {
        if (healthBar != null) {
            healthBar.value = currentHealth / maxHealth;
        }
        
        if (healthText != null) {
            healthText.text = Mathf.Round(currentHealth) + " / " + maxHealth;
        }
    }
    
    // Public methods for anti-cheat testing
    public float GetCurrentHealth() => currentHealth;
    public float GetMaxHealth() => maxHealth;
    public float GetHealthPercentage() => currentHealth / maxHealth;
    public bool IsAlive() => currentHealth > 0f;
}
```

### **EnemyHealth.cs**
```csharp
using UnityEngine;

public class EnemyHealth : MonoBehaviour {
    [Header("Health Settings")]
    public float maxHealth = 100f;
    public float currentHealth;
    
    [Header("Visual")]
    public UnityEngine.UI.Slider healthBar;
    public GameObject healthBarCanvas;
    
    [Header("ESP Testing")]
    public bool showHealthBar = true;
    public Vector3 healthBarOffset = Vector3.up * 2f;
    
    void Start() {
        currentHealth = maxHealth;
        
        // Setup health bar
        if (healthBarCanvas != null) {
            healthBarCanvas.SetActive(showHealthBar);
        }
    }
    
    void Update() {
        // Update health bar position
        if (healthBarCanvas != null) {
            healthBarCanvas.transform.position = transform.position + healthBarOffset;
            healthBarCanvas.transform.LookAt(Camera.main.transform);
        }
    }
    
    public void TakeDamage(float damage) {
        currentHealth -= damage;
        currentHealth = Mathf.Max(currentHealth, 0f);
        UpdateUI();
        
        if (currentHealth <= 0) {
            Die();
        }
    }
    
    void UpdateUI() {
        if (healthBar != null) {
            healthBar.value = currentHealth / maxHealth;
        }
    }
    
    void Die() {
        // Death logic
        Destroy(gameObject);
    }
    
    // Public methods for anti-cheat testing
    public float GetCurrentHealth() => currentHealth;
    public float GetMaxHealth() => maxHealth;
    public float GetHealthPercentage() => currentHealth / maxHealth;
    public bool IsAlive() => currentHealth > 0f;
    public bool ShouldShowHealthBar() => showHealthBar && IsAlive();
}
```

---

## 🎮 **GAME OBJECT SETUP**

### **1. Player Setup**
1. Create empty GameObject named "Player"
2. Add CharacterController component
3. Add Camera as child object
4. Add PlayerController script
5. Add PlayerHealth script
6. Add WeaponSystem script
7. Tag as "Player"

### **2. Enemy Setup**
1. Create empty GameObject named "Enemy"
2. Add NavMeshAgent component
3. Add EnemyAI script
4. Add EnemyHealth script
5. Add Capsule Collider
6. Tag as "Enemy"
7. Create health bar UI (Canvas + Slider)

### **3. Level Setup**
1. Create plane for ground
2. Add some walls/obstacles
3. Add spawn points
4. Add lighting
5. Bake NavMesh for enemy AI

---

## 🎯 **TESTING TARGETS**

### **ESP Testing Components**
```csharp
// Add this to any object for ESP testing
public class ESPTestTarget : MonoBehaviour {
    [Header("ESP Data")]
    public string displayName = "Target";
    public Vector3 boundingBoxSize = Vector3.one;
    public Color espColor = Color.red;
    public bool showESP = true;
    
    // ESP data accessors
    public Vector3 GetPosition() => transform.position;
    public Vector3 GetBoundingBox() => boundingBoxSize;
    public string GetName() => displayName;
    public Color GetColor() => espColor;
    public bool IsVisible() => showESP;
}
```

### **Aimbot Testing Components**
```csharp
// Add this to any object for aimbot testing
public class AimbotTestTarget : MonoBehaviour {
    [Header("Aimbot Data")]
    public Transform headTransform;
    public Transform chestTransform;
    public bool canBeTargeted = true;
    public float targetPriority = 1f;
    
    // Aimbot data accessors
    public Vector3 GetHeadPosition() => headTransform ? headTransform.position : transform.position + Vector3.up * 1.6f;
    public Vector3 GetChestPosition() => chestTransform ? chestTransform.position : transform.position + Vector3.up * 1f;
    public Vector3 GetCenterPosition() => transform.position + Vector3.up * 0.9f;
    public bool CanBeTargeted() => canBeTargeted;
    public float GetPriority() => targetPriority;
}
```

---

## 🚀 **QUICK DEPLOYMENT**

### **1. Create Basic Scene**
1. New Scene → "TestArena"
2. Add Player GameObject
3. Add 3-5 Enemy GameObjects
4. Add plane for ground
5. Add some walls/cover

### **2. Test Basic Functionality**
1. Press Play
2. Test movement (WASD + Mouse)
3. Test shooting (Left Click)
4. Test enemy AI
5. Test health system

### **3. Add Anti-Cheat Testing**
1. Add ESPTestTarget to enemies
2. Add AimbotTestTarget to enemies
3. Add debug panel for testing
4. Test your anti-cheat system

---

## 🎯 **NEXT STEPS**

1. **Create the Unity project**
2. **Copy the scripts above**
3. **Set up basic scene**
4. **Test functionality**
5. **Add ESP testing targets**
6. **Test your anti-cheat system**
7. **Gradually add complexity**

This gives you a **complete, safe, and legal** testing environment for your anti-cheat evasion system! 🎮
