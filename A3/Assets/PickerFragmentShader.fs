#version 330

out vec4 fragColour;

uniform uint objectID;

void main() {
	fragColour = vec4(objectID / 255.0, (objectID+20.0) / 255.0, (objectID+20.0) / 255.0, 1.0);
}
