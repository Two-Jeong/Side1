using UnityEngine;

namespace _02.Scripts.InGame
{
    public class PlayerController : MonoBehaviour
    {
        public enum PlayerState
        {
            Idle,
            Move
        }

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

        [Header("Animation")]
        [SerializeField] private Animator animator;
        [SerializeField] private string animSpeedParam = "Speed";
        [SerializeField] private string animMoveXParam = "MoveX";
        [SerializeField] private string animMoveYParam = "MoveY";
        [SerializeField] private bool keepLastDirectionOnIdle = true;

        private Rigidbody2D rb2D;
        private Vector2 input;
        private Vector2 velocity;
        private Vector2 lastMoveDir = Vector2.down;
        private PlayerState state = PlayerState.Idle;
        private bool isDashing;
        private float dashTimer;
        private float dashCooldownTimer;
        private Vector2 dashDir;

        public Vector2 Position => rb2D != null ? rb2D.position : (Vector2)transform.position;
        public Vector2 Velocity => velocity;
        public float Rotation => transform.eulerAngles.z;
        public PlayerState State => state;

        private void Awake()
        {
            rb2D = GetComponent<Rigidbody2D>();
            if (animator == null)
            {
                animator = GetComponent<Animator>();
            }
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
                return;
            }

            Vector2 dir = input;
            if (dir.sqrMagnitude > 1f)
            {
                dir = dir.normalized;
            }

            velocity = dir * moveSpeed;
        }

        private void UpdateState()
        {
            state = velocity.sqrMagnitude > 0.0001f ? PlayerState.Move : PlayerState.Idle;
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
            if (animator == null)
            {
                return;
            }

            float speed = velocity.magnitude;
            animator.SetFloat(animSpeedParam, speed);

            Vector2 dir;
            if (speed > 0.01f)
            {
                dir = velocity.normalized;
                lastMoveDir = dir;
            }
            else
            {
                dir = keepLastDirectionOnIdle ? lastMoveDir : Vector2.zero;
            }

            animator.SetFloat(animMoveXParam, dir.x);
            animator.SetFloat(animMoveYParam, dir.y);
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
