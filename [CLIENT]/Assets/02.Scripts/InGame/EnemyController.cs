using DG.Tweening;
using UnityEngine;
using _02.Scripts.InGame;

public class EnemyController : MonoBehaviour
{
    [Header("Combat")]
    [SerializeField] private int maxHp = 3;
    [SerializeField] private float hitKnockback = 0.2f;
    [SerializeField] private float hitDuration = 0.12f;
    [SerializeField] private float hitShakeStrength = 0.12f;
    [SerializeField] private float hitShakeVibrato = 12f;
    [SerializeField] private float hitShakeRandomness = 90f;
    [SerializeField] private bool flashOnHit = true;
    [SerializeField] private Color hitColor = new Color(1f, 0.5f, 0.5f, 1f);

    private int currentHp;
    private SpriteRenderer spriteRenderer;
    private Color originalColor = Color.white;
    private Tween hitTween;

    private void Awake()
    {
        currentHp = maxHp;
        spriteRenderer = GetComponentInChildren<SpriteRenderer>();
        if (spriteRenderer != null)
        {
            originalColor = spriteRenderer.color;
        }
    }

    public void TakeDamage(HitInfo info)
    {
        ApplyDamage(info.Damage, info.Direction);
    }

    public void TakeDamage(int damage)
    {
        ApplyDamage(damage, Vector2.zero);
    }

    private void ApplyDamage(int damage, Vector2 hitDir)
    {
        if (damage <= 0)
        {
            return;
        }

        currentHp -= damage;
        PlayHitTween(hitDir);

        if (currentHp <= 0)
        {
            Die();
        }
    }

    private void PlayHitTween(Vector2 hitDir)
    {
        if (hitTween != null && hitTween.IsActive())
        {
            hitTween.Kill(false);
        }

        Vector2 dir = hitDir.sqrMagnitude > 0.0001f ? hitDir.normalized : Vector2.zero;
        Vector3 targetPos = transform.position + (Vector3)(dir * hitKnockback);

        Sequence seq = DOTween.Sequence();
        if (dir.sqrMagnitude > 0.0001f && hitKnockback > 0f)
        {
            seq.Append(transform.DOMove(targetPos, hitDuration).SetEase(Ease.OutQuad));
        }
        seq.Join(transform.DOShakePosition(hitDuration, hitShakeStrength, (int)hitShakeVibrato, hitShakeRandomness, false, true));
        if (flashOnHit && spriteRenderer != null)
        {
            seq.Join(spriteRenderer.DOColor(hitColor, hitDuration * 0.5f));
            seq.Append(spriteRenderer.DOColor(originalColor, hitDuration * 0.5f));
        }

        hitTween = seq;
    }

    private void Die()
    {
        Destroy(gameObject);
    }
}
