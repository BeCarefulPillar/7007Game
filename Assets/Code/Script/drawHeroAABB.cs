using UnityEngine;

public class DrawHeroAABB: MonoBehaviour{
    public Vector3 size = new Vector3(5,5,0);
    public Color color = Color.red;

    private void Start() {
    }

    void OnDrawGizmos() {
        DrawHelper.DrawRect(new Rect(transform.position - size/2.0f , size), transform.parent.position.z, color);
    }

    public void SetCollideColor(bool isCollide) {
        if (isCollide) {
            color = Color.green;
        } else {
            color = Color.red;
        }
    }

    private void Update() {

    }

}