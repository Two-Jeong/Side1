using UnityEngine;

public class CameraController : MonoBehaviour
{
    [SerializeField] private Transform target;
    [SerializeField] private Vector3 offset = new Vector3(0f, 0f, -10f);
    [SerializeField] private float smoothTime = 0.15f;
    [SerializeField] private float maxSpeed = 0f;
    [SerializeField] private bool followInFixedUpdate = true;
    [SerializeField] private bool useTargetRigidbodyPosition = true;
    private Vector3 velocity;
    private Rigidbody2D targetRb;

    private void Start()
    {
        if (target == null)
        {
            GameObject player = GameObject.FindGameObjectWithTag("Player");
            if (player != null)
            {
                target = player.transform;
            }
        }

        CacheTargetRigidbody();
    }

    private void FixedUpdate()
    {
        if (!followInFixedUpdate)
        {
            return;
        }

        Follow(Time.fixedDeltaTime);
    }

    private void LateUpdate()
    {
        if (followInFixedUpdate)
        {
            return;
        }

        Follow(Time.deltaTime);
    }

    private void CacheTargetRigidbody()
    {
        if (target == null)
        {
            return;
        }

        targetRb = target.GetComponent<Rigidbody2D>();
    }

    private void Follow(float deltaTime)
    {
        if (target == null)
        {
            return;
        }

        if (targetRb == null && useTargetRigidbodyPosition)
        {
            CacheTargetRigidbody();
        }

        Vector3 targetPos;
        if (useTargetRigidbodyPosition && targetRb != null)
        {
            targetPos = new Vector3(targetRb.position.x, targetRb.position.y, 0f);
        }
        else
        {
            targetPos = target.position;
        }

        Vector3 desired = targetPos + offset;
        if (smoothTime <= 0f)
        {
            transform.position = desired;
            return;
        }

        float max = maxSpeed <= 0f ? Mathf.Infinity : maxSpeed;
        transform.position = Vector3.SmoothDamp(transform.position, desired, ref velocity, smoothTime, max, deltaTime);
    }
}
