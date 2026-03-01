using System.Collections.Generic;
using UnityEngine;

namespace _02.Scripts.InGame
{
    public class PlayerController : MonoBehaviour
    {
        [Header("Animation (SPUM)")]
        [SerializeField] private SPUM_Prefabs spumPrefabs;
        [SerializeField] private bool updateDepthFromY = true;
        [SerializeField] private bool flipSpriteByMoveX = true;
        [SerializeField] private int idleAnimIndex = 0;
        [SerializeField] private int moveAnimIndex = 0;
        [SerializeField] private int attackAnimIndex = 0;

        [Header("Combat")]
        [SerializeField] private int attackDamage = 1;
        [SerializeField] private float attackRange = 1f;
        [SerializeField] private float attackRadius = 0.5f;
        [SerializeField] private float attackDuration = 0.2f;
        [SerializeField] private float attackCooldown = 0.4f;
        [SerializeField] private LayerMask attackLayers;
        [SerializeField] private bool drawAttackGizmo = true;

        private PlayerMoveMent movement;
        private readonly Dictionary<PlayerState, int> animIndex = new();
        private bool isAttacking;
        private float attackTimer;
        private float attackCooldownTimer;
        private bool attackHitTriggered;
        private Vector2 attackDir = Vector2.right;
        private PlayerState lastPlayedState = PlayerState.IDLE;

        private void Start()
        {
            movement = GetComponent<PlayerMoveMent>();
            InitSpumAnimation();
        }

        private void Update()
        {
            UpdateAttackTimers(Time.deltaTime);

            if (Input.GetMouseButtonDown(0))
            {
                TryStartAttack();
            }

            UpdateAnimation();
        }

        private void UpdateAnimation()
        {
            if (spumPrefabs == null || movement == null)
            {
                return;
            }

            if (updateDepthFromY)
            {
                transform.position = new Vector3(transform.position.x, transform.position.y, transform.localPosition.y * 0.01f);
            }

            Vector2 faceDir = isAttacking ? attackDir : movement.LastMoveDir;
            if (flipSpriteByMoveX && faceDir.sqrMagnitude > 0.0001f)
            {
                spumPrefabs.transform.localScale = new Vector3(faceDir.x > 0f ? -1f : 1f, 1f, 1f);
            }

            PlayerState nextState = isAttacking ? PlayerState.ATTACK : movement.State;
            if (nextState != lastPlayedState)
            {
                PlayStateAnimation(nextState);
                lastPlayedState = nextState;
            }
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
            animIndex[PlayerState.ATTACK] = attackAnimIndex;
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

        private void UpdateAttackTimers(float deltaTime)
        {
            if (attackCooldownTimer > 0f)
            {
                attackCooldownTimer -= deltaTime;
            }

            if (!isAttacking)
            {
                return;
            }

            attackTimer -= deltaTime;
            if (attackTimer <= 0f)
            {
                if (!attackHitTriggered)
                {
                    attackHitTriggered = true;
                    PerformAttack(attackDir);
                }

                isAttacking = false;
            }
        }

        private void TryStartAttack()
        {
            if (isAttacking || attackCooldownTimer > 0f)
            {
                return;
            }

            Camera cam = Camera.main;
            if (cam == null)
            {
                return;
            }

            Vector2 mouseWorld = cam.ScreenToWorldPoint(Input.mousePosition);
            Vector2 dir = mouseWorld - (Vector2)transform.position;
            if (dir.sqrMagnitude < 0.0001f)
            {
                dir = movement != null ? movement.LastMoveDir : Vector2.right;
            }

            attackDir = dir.normalized;
            isAttacking = true;
            attackTimer = attackDuration;
            attackCooldownTimer = attackCooldown;
            attackHitTriggered = false;
        }

        private void PerformAttack(Vector2 dir)
        {
            Vector2 center = (Vector2)transform.position + dir * attackRange;
            Collider2D[] hits = Physics2D.OverlapCircleAll(center, attackRadius, attackLayers);
            HitInfo info = new HitInfo(attackDamage, dir);
            for (int i = 0; i < hits.Length; i++)
            {
                hits[i].SendMessage("TakeDamage", info, SendMessageOptions.DontRequireReceiver);
            }
        }

        private void OnDrawGizmosSelected()
        {
            if (!drawAttackGizmo)
            {
                return;
            }

            Vector2 dir = attackDir.sqrMagnitude > 0.0001f ? attackDir : Vector2.right;
            Vector2 center = (Vector2)transform.position + dir * attackRange;
            Gizmos.color = Color.red;
            Gizmos.DrawWireSphere(center, attackRadius);
        }
    }
}
