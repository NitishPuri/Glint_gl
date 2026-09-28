#version 330 core

in vec2 UV;

out vec3 color;

uniform sampler2D renderedTexture;
uniform float time;
uniform bool isDepth;

// depth visualization!
uniform float depthNear = 0.1;
uniform float depthFar = 100.0;
uniform float depthScale = 1.0;

float linearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Convert from [0,1] to [-1,1]
    return (2.0 * depthNear * depthFar) / (depthFar + depthNear - z * (depthFar - depthNear));
}

// ways to visualize depth, none of them seem to work very well.
float remapDepth(float depth) {
    // First linearize the depth
    float linearDepth = linearizeDepth(depth);
    // Remap to 0-1 range based on near/far planes
    float remappedDepth = (linearDepth - depthNear) / (depthFar - depthNear);
    // Apply scaling to enhance visibility
    // return clamp(remappedDepth * depthScale, 0.0, 1.0);
	    
    // Method 1: Power function to enhance contrast
    // float enhancedDepth = pow(linearDepth, depthScale);
    // float enhancedDepth = pow(linearDepth, 4);

	// Method 2: Exponential enhancement
	// float enhancedDepth = 1.0 - exp(-linearDepth * depthScale);

	// Method 3: Custom range mapping with enhanced mid-tones
    float midPoint = (depthNear + depthFar) * 0.5;
    float enhancedDepth = smoothstep(depthNear, depthFar, linearDepth * depthScale);

	return clamp(enhancedDepth, 0.0, 1.0);

}

void main(){
	vec2 wobblyUV = UV;

	wobblyUV.x += 0.005*sin(time+1024.0*UV.x);
	wobblyUV.y += 0.005*cos(time+768.0*UV.y);

	wobblyUV.x += sin(UV.y * 10.0 + time) * 0.01;

	if(isDepth){
		float depth = texture(renderedTexture, wobblyUV).r;
		depth = remapDepth(depth);		
		color = vec3(depth);				
	} else {
		color = texture( renderedTexture, wobblyUV ).xyz ;
	}
}