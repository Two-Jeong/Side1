using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class UI_CreateAccountPopup : UI_BasePopup
{
    public override bool IsDontTouchField => true;

    [SerializeField] private UIButton btnClose;
    [SerializeField] private UIButton btnCreate;

    [Header("Input Fields")]
    [SerializeField] private TMP_InputField idInputField;
    [SerializeField] private TMP_InputField passwordInputField;

    public void Awake()
    {
        btnClose.onClick.AddListener(OnBtnCloseClick);
        btnCreate.onClick.AddListener(OnBtnCreateClick);
    }

    private void OnBtnCloseClick()
    {
        Close();
    }

    private void OnBtnCreateClick()
    {
        C2S_AccountRegister register_request_packet = new C2S_AccountRegister();
        register_request_packet.Id = idInputField.text;
        register_request_packet.Password = passwordInputField.text;
        NetworkManager.Instance.DoSend(register_request_packet);
    }

    public void Show()
    {
        idInputField.text = "";
        passwordInputField.text = "";
    }
}
