# Deferred Renderer

This is the speedup branch, rough speeds for unoptimised version is
|-------|-----|
|Geo pass |1870|
|SSAO |50|
|Lighting |160
|Forward |170|
|Bloom Blur |123|
|Final |80|
|Total |126800|
|135.2 |  7.4 FPS|


Pass 1 collect Position normal and albedoSpec
Pass 2 Lighting Pass write to m_lightingFBO fragColour (from lighting) and brightness
Pass 3 copy this to FBO for forward render of lights

This uses multiple FBOs

m_renderFBO is the first pass and has the following outputs

```
ATTACHMENT_0 position
ATTACHMENT_1 normal
ATTACHMENT_2 albedoSpec

```
m_lightingFBO
```
ATTACHMENT_0 fragColour
ATTACHMENT_1 brightness
```



