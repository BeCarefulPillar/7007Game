Shader "Test/sf1" //不用编写pass通道
{
    Properties
    {
        _Color ("Color", Color) = (1,1,1,1)
        _MainTex ("Albedo (RGB)", 2D) = "white" {}
        _Glossiness ("Smoothness", Range(0,1)) = 0.5 //高光
        _Metallic ("Metallic", Range(0,1)) = 0.0     //金属光泽
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }  //渲染类型 不透明的物体
        LOD 200                         //层级细节

        CGPROGRAM
        // Physically based Standard lighting model, and enable shadows on all light types
        #pragma surface surf Standard fullforwardshadows //编译指令 关键词surface 函数名 光照模型 其他选项

        // Use shader model 3.0 target, to get nicer looking lighting
        #pragma target 3.0 //更好的光照

        struct Input
        {
            float2 uv_MainTex;  //必须以uv开头  uv_ 第一套uv, uv2_ 第二套uv
        };
        //声明 并不是和属性的类型是一一对应的
        sampler2D _MainTex;
        half _Glossiness;
        half _Metallic;
        fixed4 _Color;

        // Add instancing support for this shader. You need to check 'Enable Instancing' on materials that use the shader.
        // See https://docs.unity3d.com/Manual/GPUInstancing.html for more information about instancing.
        // #pragma instancing_options assumeuniformscaling
        UNITY_INSTANCING_BUFFER_START(Props)
            // put more per-instance properties here
        UNITY_INSTANCING_BUFFER_END(Props)
        //inout 输入输出 默认为in 
        void surf (Input IN, inout SurfaceOutputStandard o) //总是无返回的
        {
            // Albedo comes from a texture tinted by color
            fixed4 c = tex2D (_MainTex, IN.uv_MainTex) * _Color; //tex2D纹理采样
            o.Albedo = c.rgb;
            // Metallic and smoothness come from slider variables
            o.Metallic = _Metallic;
            o.Smoothness = _Glossiness;
            o.Alpha = c.a;
        }
        ENDCG
    }
    FallBack "Diffuse"
}
