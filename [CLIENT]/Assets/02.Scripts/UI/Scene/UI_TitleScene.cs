using UnityEngine;

public class UI_TitleScene : MonoBehaviour
{
    [SerializeField] private UIButton btnCreateAccount;
    [SerializeField] private UIButton btnLogin;

    public void Awake()
    {
        btnCreateAccount.onClick.AddListener(OnBtnCreateAccountClick);
        btnLogin.onClick.AddListener(OnBtnLoginClick);
    }

    private void OnBtnCreateAccountClick()
    {
        UIManager.Instance.OpenPopup<UI_CreateAccountPopup>().Show();
    }

    private void OnBtnLoginClick()
    {
        
    }

    // Start is called once before the first execution of Update after the MonoBehaviour is created
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
