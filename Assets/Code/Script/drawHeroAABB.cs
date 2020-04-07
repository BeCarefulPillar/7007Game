
using UnityEngine;

public class DrawHeroAABB: MonoBehaviour{
    private MeshRenderer mesh;
    public Vector3 size;

    private void Start() {
        foreach (Transform child in transform.GetComponentsInChildren<Transform>()) {
            mesh = child.GetComponent<MeshRenderer>();
            if (mesh != null) {
                Debug.Log(mesh.bounds.size);
            }
        }
    }

    void LateUpdate() {
        if (mesh != null) {
            DrawHelper.DrawRect(new Rect(transform.position - size/2.0f , size), transform.parent.position.z);
        }
    }
}