using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class UI_TitleScene : MonoBehaviour
{
    [Header("Buttons")]
    [SerializeField] private UIButton btnCreateAccount;
    [SerializeField] private UIButton btnLogin;

    [Header("Input Fields")]
    [SerializeField] private TMP_InputField idInputField;
    [SerializeField] private TMP_InputField passwordInputField;

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
        C2S_AccountLogin login_request_packet = new C2S_AccountLogin();
        login_request_packet.Id = idInputField.text;
        login_request_packet.Password = passwordInputField.text;

        NetworkManager.Instance.DoSend(login_request_packet);
    }
}
