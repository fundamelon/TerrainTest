#version 400
 
// triangles, quads, or isolines
layout (triangles, equal_spacing, cw) in;
in vec3 evaluationpoint_wor[];
 
// could use a displacement map here
 
uniform mat4 viewmat;
uniform mat4 projmat;
 
// gl_TessCoord is location within the patch
// (barycentric for triangles, UV for quads)
 
void main () {
  vec3 p0 = gl_TessCoord.x * evaluationpoint_wor[0]; // x is one corner
  vec3 p1 = gl_TessCoord.y * evaluationpoint_wor[1]; // y is the 2nd corner
  vec3 p2 = gl_TessCoord.z * evaluationpoint_wor[2]; // z is the 3rd corner (ignore when using quads)
  vec3 pos = normalize (p0 + p1 + p2);
  gl_Position = projmat * viewmat * vec4 (pos, 1.0);
}