using UnityEngine;

public class CameraFollow : MonoBehaviour
{
    [Header("추적 대상")]
    public Transform target;       // 따라다닐 플레이어

    [Header("설정")]
    public float smoothTime = 0.25f; // 지연 시간 (낮을수록 빨리 따라감, 높을수록 부드러움)
    public Vector3 offset;           // 플레이어와의 거리 (자동 설정 가능)
    
    [Header("옵션")]
    public bool useInitialOffset = true; // 게임 시작 시 현재 카메라 위치를 오프셋으로 쓸지 여부

    private Vector3 velocity = Vector3.zero; // SmoothDamp 계산용 변수

    void Start()
    {
        if (target == null)
        {
            Debug.LogWarning("CameraFollow: Target이 설정되지 않았습니다!");
            return;
        }

        // 게임 시작 시, 현재 에디터에 배치된 카메라와 플레이어의 간격을 그대로 유지
        if (useInitialOffset)
        {
            offset = transform.position - target.position;
            offset.x = 0;
        }
    }

    // 플레이어 이동은 FixedUpdate나 Update에서 일어나므로, 
    // 카메라는 그 이후인 LateUpdate에서 움직여야 떨림(Jitter)이 없습니다.
    void LateUpdate()
    {
        if (target == null) return;

        // 1. 목표 위치 계산
        Vector3 targetPosition = target.position + offset;

        // 2. 부드럽게 이동 (SmoothDamp)
        // 현재 위치에서 목표 위치까지 smoothTime 동안 부드럽게 이동
        transform.position = Vector3.SmoothDamp(transform.position, targetPosition, ref velocity, smoothTime);
    }
}