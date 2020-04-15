
using UnityEngine;

public static class DrawHelper {
    public static void DrawRect(Rect rect, float z, Color color) {
        Vector3[] line = new Vector3[5];

        line[0] = new Vector3(rect.x, rect.y, z);

        line[1] = new Vector3(rect.x + rect.width, rect.y, z);

        line[2] = new Vector3(rect.x + rect.width, rect.y + rect.height, z);

        line[3] = new Vector3(rect.x, rect.y + rect.height, z);

        line[4] = new Vector3(rect.x, rect.y, z);

        if (line != null && line.Length > 0) {
            DrawLineHelper(line, color);
        }

    }

    public static void DrawLineHelper(Vector3[] line, Color color) {
        Gizmos.color = color;
        for (var i = 0; i < line.Length - 1; i++) {
            Gizmos.DrawLine(line[i], line[i + 1]);
        }
    }
}
