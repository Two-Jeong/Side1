using System.Collections; // 코루틴 사용을 위해 필요
using UnityEngine;

public class BeltScrollMovement : MonoBehaviour
{
    [Header("설정")]
    public float moveSpeed = 6f;      
    public float jumpForce = 8f;      
    public float dashSpeed = 12f;     
    public float dashDuration = 1.0f; // [추가] 대쉬 지속 시간 (이동 불가 시간)
    
    [Header("컴포넌트")]
    public LayerMask groundLayer;     
    public Animator anim;
    public Transform shadowObject;

    private Rigidbody rb;
    private Vector3 movementInput;
    private bool isGrounded;
    private bool isDashing = false;   // [추가] 현재 대쉬 중인지 확인하는 플래그
    private float groundCheckDistance = 0.5f;

    void Start()
    {
        rb = GetComponent<Rigidbody>();
        rb.freezeRotation = true; 
        shadowObject.rotation = transform.rotation;
    }

    void Update()
    {
        // [중요] 대쉬 중이라면 입력을 받지 않고 리턴(함수 종료)
        if (isDashing) return;

        // --- 1. 일반 이동 입력 ---
        float inputX = Input.GetAxisRaw("Horizontal");
        float inputZ = Input.GetAxisRaw("Vertical");

        movementInput = new Vector3(inputX, 0f, inputZ).normalized;

        // --- 2. 방향 전환 ---
        if (inputX != 0)
        {
            // 오른쪽(0도), 왼쪽(-180도)
            float targetAngle = inputX > 0 ? 0 : -180f;
            transform.rotation = Quaternion.Euler(0f, targetAngle, 0f);
        }

        // --- 3. 점프 ---
        CheckGround();
        if (Input.GetKeyDown(KeyCode.Space) && isGrounded)
        {
            rb.AddForce(Vector3.up * jumpForce, ForceMode.Impulse);
            anim.SetTrigger("DoJump");
        }

        // --- 4. 대쉬 입력 ---
        // (쿨타임 방지 등을 위해 isDashing이 false일 때만 실행)
        if (Input.GetKeyDown(KeyCode.K) && !isDashing)
        {
            StartCoroutine(DashRoutine());
        }

        if (Input.GetKeyDown(KeyCode.J))
        {
            anim.SetTrigger("DoAttack");
        }

        UpdateAnimation();
    }

    void FixedUpdate()
    {
        // [중요] 대쉬 중일 때는 일반 이동 물리 연산을 막음
        // (이걸 안 막으면 대쉬 힘(Force)이 가해지자마자 이 함수가 속도를 0으로 덮어씌워 버림)
        if (isDashing) return;

        MovePlayer();
    }

    void LateUpdate()
    {
        if (shadowObject == null) return;

        // 캐릭터 위치에서 아래로 긴 레이저를 쏩니다.
        // (점프를 높게 해도 바닥을 감지할 수 있도록 충분히 길게 쏩니다. 예: 50f)
        RaycastHit hit;
        Vector3 rayOrigin = transform.position + Vector3.up * 0.5f; // 캐릭터 중심 약간 위에서 시작

        if (Physics.Raycast(rayOrigin, Vector3.down, out hit, 50f, groundLayer))
        {
            // 바닥을 감지했다면 그림자를 표시합니다.
            shadowObject.gameObject.SetActive(true);

            // 그림자의 목표 위치 계산:
            // X, Z는 플레이어의 위치를 그대로 사용
            // Y는 레이저가 맞은 바닥의 위치(hit.point.y)를 사용
            Vector3 targetShadowPos = new Vector3(transform.position.x, hit.point.y, transform.position.z);
            
            // 바닥과 완전히 겹치면 깜빡거릴 수 있으므로 아주 살짝 위로 띄웁니다 (+0.01f)
            shadowObject.position = targetShadowPos + Vector3.up * 0.05f;
        }
        else
        {
            // 캐릭터가 너무 높이 있거나 바닥이 없는 곳(낭떠러지) 위에 있다면 그림자를 숨깁니다.
            shadowObject.gameObject.SetActive(false);
        }
    }

    void MovePlayer()
    {
        Vector3 targetVelocity = movementInput * moveSpeed;
        // Unity 버전에 따라 rb.velocity 또는 rb.linearVelocity 사용
        rb.linearVelocity = new Vector3(targetVelocity.x, rb.linearVelocity.y, targetVelocity.z);
    }

    void CheckGround()
    {
        Vector3 origin = transform.position;
        isGrounded = Physics.Raycast(origin, Vector3.down, groundCheckDistance + 0.5f, groundLayer);
    }

    void UpdateAnimation()
    {
        bool isMoving = movementInput.sqrMagnitude > 0.01f;
        anim.SetBool("IsMove", isMoving);
        anim.SetBool("IsGrounded", isGrounded);
    }

    // [추가] 대쉬 처리 코루틴
    IEnumerator DashRoutine()
    {
        isDashing = true;
        
        // 1. 대쉬 애니메이션 실행
        anim.SetTrigger("DoDash");

        rb.linearVelocity = Vector3.zero;
        rb.angularVelocity = Vector3.zero;

        // 2. 대쉬 물리 힘 가하기
        // 캐릭터가 바라보는 방향(transform.right)으로 힘을 가함
        // (캐릭터가 0도면 오른쪽, -180도면 transform.right가 왼쪽을 가리킴)
        int dir = transform.rotation.y == 0 ? 1 : -1;
        rb.AddForce(Vector3.right * dashSpeed * dir, ForceMode.Impulse);

        // 3. 지정된 시간만큼 대기 (여기서 1초간 멈춤)
        yield return new WaitForSeconds(dashDuration);

        // 4. 대쉬 종료 처리
        isDashing = false; // 상태 잠금 해제
        
        rb.linearVelocity = Vector3.zero; 
    }
}