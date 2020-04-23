using UnityEngine;
using UnityEngine.UI;

public class DrawHud : MonoBehaviour {
    public GameObject obj;
    public Font font;
    public Camera sceneCamera;
    public Camera uiCamera;
    public RectTransform parent;

    private Text txt;
    Vector2 retPos;
    private void Start() {
        GameObject hud = GameObject.Instantiate(new GameObject(), obj.transform);
        txt = hud.AddComponent<Text>();
        txt.text = "22222222222";
        txt.font = font;                  
    }

    private void LateUpdate() {
        var screenPos = sceneCamera.WorldToScreenPoint(transform.position);
        Vector2 tempV2 = new Vector2(screenPos.x, screenPos.y);
        RectTransformUtility.ScreenPointToLocalPointInRectangle(parent, tempV2, uiCamera, out retPos);
        txt.transform.localPosition = retPos;
    }

}