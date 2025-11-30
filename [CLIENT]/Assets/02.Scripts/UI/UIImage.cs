using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class UIImage : MonoBehaviour
{
    [System.Serializable]
    public struct MinSizeElement
    {
        public Vector2 min;
        public Vector2 max;

        public Vector2 setSize;
    }

    public Image image;
    [SerializeField] private Vector2 baseSize;

    [SerializeField] private bool useMinSize = false;

    [SerializeField] private MinSizeElement[] minSizeElements;

    [SerializeField] private bool useUVCenter = false;

    public void Reset()
    {
        image = GetComponent<Image>();
        baseSize = image.rectTransform.sizeDelta;
    }

    public bool UseMinSize { set { useMinSize = value; } }

    public Sprite sprite
    {
        get
        {
            return image.sprite;
        }
        set
        {
            image.sprite = value;

            image.SetNativeSize();

            if (useUVCenter)
                SetUVCenter();

            if (baseSize.x != 0f && baseSize.y != 0f)
            {
                Vector2 textureSize = image.sprite != null ? image.sprite.rect.size : image.rectTransform.sizeDelta;
                bool setSize = false;
                if (useMinSize)
                {
                    if (minSizeElements.Length > 0)
                    {
                        foreach (var minSize in minSizeElements)
                        {
                            if ((textureSize.x > minSize.min.x && textureSize.x <= minSize.max.x) ||
                                (textureSize.y > minSize.min.y && textureSize.y <= minSize.max.y))
                            {
                                float multiple;

                                if (textureSize.x < textureSize.y)
                                {
                                    float deltaX = minSize.setSize.x / textureSize.x;
                                    multiple = deltaX;
                                }
                                else
                                {
                                    float deltaY = minSize.setSize.y / textureSize.y;
                                    multiple = deltaY;
                                }

                                image.rectTransform.sizeDelta = textureSize * multiple;
                                setSize = true;

                                if (image.rectTransform.sizeDelta.x > baseSize.x || image.rectTransform.sizeDelta.y > baseSize.y)
                                    setSize = false;

                                break;
                            }
                        }
                    }
                }

                if (setSize == false)
                {
                    float multiple;

                    float deltaX = baseSize.x / textureSize.x;
                    float deltaY = baseSize.y / textureSize.y;

                    if (deltaX < deltaY)
                        multiple = deltaX;
                    else
                        multiple = deltaY;

                    image.rectTransform.sizeDelta = textureSize * multiple;
                }
            }
        }
    }

    [ContextMenu("SetUVCenter")]
    public void SetUVCenter()
    {
        Vector2 beforePos = image.rectTransform.anchoredPosition;
        image.rectTransform.pivot = new Vector2(image.sprite.pivot.x / image.sprite.rect.size.x, image.sprite.pivot.y / image.sprite.rect.size.y);
        image.rectTransform.anchoredPosition = beforePos;
    }
}
