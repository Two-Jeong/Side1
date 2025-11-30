using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using System;

public class UIManager : Singleton<UIManager>
{
    protected override bool DontDestroyOnload => true;
    private readonly string PathPopup = "Popups/{0}";

    [SerializeField] private Canvas popupCanvas;

    private Dictionary<Type, UI_BasePopup> popups = new Dictionary<Type, UI_BasePopup>();

    private List<UI_BasePopup> openPopups = new List<UI_BasePopup>();

    public bool IsOpenPopup(UI_BasePopup _except)
    {
        return popups.Any(x => x.Value.IsOpen && x.Value != _except);
    }

    public bool IsOpenPopup()
    {
        return popups.Any(x => x.Value.IsOpen);
    }

    public bool IsOpenPopup<T>(out T _popup) where T : UI_BasePopup
    {
        Type popupType = typeof(T);
        _popup = null;

        if (popups.ContainsKey(popupType) == false)
            return false;

        _popup = popups[popupType] as T;

        return popups[popupType].IsOpen;
    }

    public bool IsDontTouchField()
    {
        return popups.Any(x => x.Value.IsDontTouchField && x.Value.IsOpen);
    }

    public T GetPopup<T>() where T : UI_BasePopup
    {
        Type popupType = typeof(T);

        var popup = openPopups.Find(x => x is T);
        if (popup == null)
            return null;

        return popup as T;
    }

    public int CountOpenPopup()
    {
        return openPopups.Count;
    }

    /// <summary> 팝업을 보여주는 함수 </summary>
    public T OpenPopup<T>() where T : UI_BasePopup
    {
        Type popupType = typeof(T);

        if (popups.ContainsKey(popupType) == false)
            CreatePopup<T>(popupType.ToString());

        if (popups[popupType].IsCloseOtherPopups)
        {
            var popups = new List<UI_BasePopup>(openPopups);
            popups.ForEach(x => { x.Close(); });
        }

        popups[popupType].Open();
        popups[popupType].transform.SetAsLastSibling();

        return popups[popupType] as T;
    }


    public void ClosePopup<T>() where T : UI_BasePopup
    {
        Type popupType = typeof(T);

        if (popups.ContainsKey(popupType) == false)
            return;

        popups[popupType].Close();
    }

    public void CloseAllPopup()
    {
        int count = openPopups.Count;
        for (int i = count - 1; i >= 0; i--)
        {
            openPopups[i].Close();
        }
    }

    /// <summary>
    /// 팝업을 생성하는 함수
    /// </summary>
    private GameObject CreatePopup<T>(string _popupName) where T : UI_BasePopup
    {
        string popupName = _popupName.Split('.').Last();

        GameObject popupGo = Resources.Load<GameObject>(string.Format(PathPopup, popupName));
        if (popupGo == null)
        {
            DebugLog.LogError($"팝업 경로가 잘못되었거나 없습니다. popupName: {popupName}");
            return null;
        }

        T popup = Instantiate(popupGo, popupCanvas.transform).GetComponent<T>();
        if (popup == null)
        {
            DebugLog.LogError($"{popupName} 컴포넌트가 존재하지 않습니다.");
            return null;
        }

        Type popupType = typeof(T);
        popups.Add(popupType, popup);

        //LocalizationManager.Instance.Refresh();

        return popupGo;
    }

    public void AddOpenPopup(UI_BasePopup _popup)
    {
        if (openPopups.Contains(_popup))
            openPopups.Remove(_popup);

        openPopups.Add(_popup);
    }

    public void RemoveOpenPopup(UI_BasePopup _popup)
    {
        if (openPopups.Contains(_popup))
            openPopups.Remove(_popup);
    }

    private void Update()
    {
        CheckBackButton();
    }

    private void CheckBackButton()
    {
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            PopupCloseByBackButton();
        }
    }

    private void PopupCloseByBackButton()
    {
        if (openPopups.Count <= 0)
            return;

        var lastPopup = openPopups.Last();

        if (lastPopup.IsDontCloseBackButton)
            return;

        if (lastPopup.IsCheckClose)
        {
            lastPopup.CheckClose();
            return;
        }

        lastPopup.Close();
    }
}
