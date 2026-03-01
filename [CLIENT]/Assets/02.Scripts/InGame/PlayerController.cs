using System.Collections.Generic;
using UnityEngine;

namespace _02.Scripts.InGame
{
    public class PlayerController : MonoBehaviour
    {
        [Header("Movement")]
        [SerializeField] private float moveSpeed = 5f;
        [SerializeField] private bool rotateToMove = true;
        [SerializeField] private float rotationOffset = 0f;
        [SerializeField] private float rotationLerpSpeed = 0f;
        [SerializeField] private bool useExternalInput = false;
        [SerializeField] private KeyCode dashKey = KeyCode.Space;
        [SerializeField] private float dashSpeed = 10f;
        [SerializeField] private float dashDuration = 0.15f;
        [SerializeField] private float dashCooldown = 0.5f;

        [Header("Animation (SPUM)")]
        [SerializeField] private SPUM_Prefabs spumPrefabs;
        [SerializeField] private bool updateDepthFromY = true;
        [SerializeField] private bool flipSpriteByMoveX = true;
        [SerializeField] private int idleAnimIndex = 0;
        [SerializeField] private int moveAnimIndex = 0;

        private Rigidbody2D rb2D;
        private Vector2 input;
        private Vector2 velocity;
        private Vector2 lastMoveDir = Vector2.down;
        private PlayerState state = PlayerState.IDLE;
        private bool isDashing;
        private float dashTimer;
        private float dashCooldownTimer;
        private Vector2 dashDir;
        private readonly Dictionary<PlayerState, int> animIndex = new();

        public Vector2 Position => rb2D != null ? rb2D.position : (Vector2)transform.position;
        public Vector2 Velocity => velocity;
        public float Rotation => transform.eulerAngles.z;
        public PlayerState State => state;

        private void Awake()
        {
            rb2D = GetComponent<Rigidbody2D>();
        }

        private void Start()
        {
            InitSpumAnimation();
        }

        private void Update()
        {
            UpdateDashTimers(Time.deltaTime);

            if (Input.GetKeyDown(dashKey))
            {
                TryStartDash();
            }

            if (!useExternalInput)
            {
                ReadInput();
            }

            UpdateVelocityFromInput();
            UpdateState();
            UpdateRotation(Time.deltaTime);
            UpdateAnimation();
        }

        private void FixedUpdate()
        {
            Vector2 delta = velocity * Time.fixedDeltaTime;

            if (rb2D != null)
            {
                rb2D.MovePosition(rb2D.position + delta);
            }
            else
            {
                transform.position += (Vector3)delta;
            }
        }

        private void ReadInput()
        {
            float x = Input.GetAxisRaw("Horizontal");
            float y = Input.GetAxisRaw("Vertical");
            input = new Vector2(x, y);
        }

        private void UpdateVelocityFromInput()
        {
            if (isDashing)
            {
                velocity = dashDir * dashSpeed;
                lastMoveDir = dashDir;
                return;
            }

            Vector2 dir = input;
            if (dir.sqrMagnitude > 1f)
            {
                dir = dir.normalized;
            }

            velocity = dir * moveSpeed;
            if (dir.sqrMagnitude > 0.0001f)
            {
                lastMoveDir = dir;
            }
        }

        private void UpdateState()
        {
            state = velocity.sqrMagnitude > 0.0001f ? PlayerState.MOVE : PlayerState.IDLE;
        }

        private void UpdateRotation(float deltaTime)
        {
            if (!rotateToMove)
            {
                return;
            }

            if (velocity.sqrMagnitude <= 0.0001f)
            {
                return;
            }

            float targetAngle = Mathf.Atan2(velocity.y, velocity.x) * Mathf.Rad2Deg + rotationOffset;
            float currentAngle = transform.eulerAngles.z;

            if (rotationLerpSpeed > 0f)
            {
                float newAngle = Mathf.MoveTowardsAngle(currentAngle, targetAngle, rotationLerpSpeed * deltaTime);
                transform.rotation = Quaternion.Euler(0f, 0f, newAngle);
            }
            else
            {
                transform.rotation = Quaternion.Euler(0f, 0f, targetAngle);
            }
        }

        private void UpdateAnimation()
        {
            if (spumPrefabs == null)
            {
                return;
            }

            if (updateDepthFromY)
            {
                transform.position = new Vector3(transform.position.x, transform.position.y, transform.localPosition.y * 0.01f);
            }

            if (flipSpriteByMoveX && lastMoveDir.sqrMagnitude > 0.0001f)
            {
                spumPrefabs.transform.localScale = new Vector3(lastMoveDir.x > 0f ? -1f : 1f, 1f, 1f);
            }

            PlayStateAnimation(state);
        }

        private void UpdateDashTimers(float deltaTime)
        {
            if (dashCooldownTimer > 0f)
            {
                dashCooldownTimer -= deltaTime;
            }

            if (!isDashing)
            {
                return;
            }

            dashTimer -= deltaTime;
            if (dashTimer <= 0f)
            {
                isDashing = false;
            }
        }

        private void TryStartDash()
        {
            if (isDashing || dashCooldownTimer > 0f)
            {
                return;
            }

            Vector2 dir = lastMoveDir;
            if (dir.sqrMagnitude < 0.0001f)
            {
                dir = input.sqrMagnitude > 0.0001f ? input.normalized : Vector2.down;
            }

            dashDir = dir.normalized;
            isDashing = true;
            dashTimer = dashDuration;
            dashCooldownTimer = dashCooldown;
        }

        private void InitSpumAnimation()
        {
            if (spumPrefabs == null && transform.childCount > 0)
            {
                spumPrefabs = transform.GetChild(0).GetComponent<SPUM_Prefabs>();
            }

            if (spumPrefabs == null)
            {
                return;
            }

            if (!spumPrefabs.allListsHaveItemsExist())
            {
                spumPrefabs.PopulateAnimationLists();
            }

            spumPrefabs.OverrideControllerInit();
            animIndex[PlayerState.IDLE] = idleAnimIndex;
            animIndex[PlayerState.MOVE] = moveAnimIndex;
        }

        public void SetStateAnimationIndex(PlayerState animState, int index = 0)
        {
            animIndex[animState] = index;
        }

        private void PlayStateAnimation(PlayerState animState)
        {
            if (!animIndex.ContainsKey(animState))
            {
                animIndex[animState] = 0;
            }

            spumPrefabs.PlayAnimation(animState, animIndex[animState]);
        }

        public void SetExternalInput(Vector2 newInput)
        {
            useExternalInput = true;
            input = newInput;
        }

        public void ClearExternalInput()
        {
            useExternalInput = false;
        }
    }
}
