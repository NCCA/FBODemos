# Deferred Renderer

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



