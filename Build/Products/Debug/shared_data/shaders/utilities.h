

//
//  utilities.nshdr.h
//  Nanocat
//
//  Created by Kawaii Neko on 12/6/15.
//  Copyright ¬© 2015 Neko Vision. All rights reserved.
//


#define PI  3.14159265
#define PI2 PI * 2.0

#define EPSILON                 10e-5f

/**
 *  Basic function to flip texture coordinates.
 */
vec2 flipTexcoords( in vec2 texcoords ) {
    return -texcoords + 1.0;
}

// Sphere map normal decode.
vec3 decode (vec4 enc)
{
    vec4 nn = enc*vec4(2.0,2.0,0.0,0.0) + vec4(-1.0,-1.0,1.0,-1.0);
    float l = dot(nn.xyz,-nn.xyw);
    nn.z = l;
    nn.xy *= sqrt(l);
    return nn.xyz * 2.0 + vec3(0.0,0.0,-1.0);
}

// Approximate conversion.
vec3 approximateSRGBToLinear(vec3 c) {
    return pow( c, vec3(2.2, 2.2, 2.2) );
}

vec3 approximateLinearToSRGB(vec3 c) {
    return pow( c, vec3(0.4545, 0.4545, 0.4545) );
}

// Accurate conversion.
// The approximation to sRGB can not create true black ( Moving Frostbite to PBR - Figure 76 ).
//  SRGB color to Linear.
vec3 accurateSRGBToLinear(in vec3 sRGBCol)
{
    vec3 linearRGBLo = sRGBCol / vec3(12.92);
    vec3 linearRGBHi = pow((sRGBCol + vec3(0.055, 0.055, 0.055)) / 1.055, vec3(2.4));
    vec3 linearRGB = (sRGBCol.x <= 0.04045 && sRGBCol.y <= 0.04045 && sRGBCol.z <= 0.04045) ? linearRGBLo : linearRGBHi;
    
    return linearRGB;
}

//  Linear color to SRGB;
vec3 accurateLinearToSRGB(in vec3 linearCol)
{
    vec3 sRGBLo = linearCol * 12.92;
    vec3 sRGBHi = (pow(abs(linearCol), vec3(1.0/2.4)) * 1.055) - vec3(0.055, 0.055, 0.055);
    vec3 sRGB = (linearCol.x <= 0.0031308 && linearCol.y <= 0.0031308 && linearCol.z <= 0.0031308) ? sRGBLo : sRGBHi;
    
    return sRGB;
}

/**
 *  Linear color to gamma corrected color.
 */
vec4 Linear2Gamma(in vec4 color) {
    return vec4(sqrt(color.r), sqrt(color.g), sqrt(color.b), color.a);
}

vec3 toGamma(in vec3 v) {
    return pow(v, vec3(1.0 / 2.2));
}

/**
 *  Linearize depth.
 */
float ld(in float depth) {
    return (2.0 * ZNEAR) / (ZFAR + ZNEAR - depth * (ZFAR - ZNEAR));
}

// Derivatives of light-space depth with respect to texture2D coordinates
vec2 depthGradient(vec2 uv, float z)
{
    vec2 dz_duv = vec2(0.0, 0.0);
    
    vec3 duvdist_dx = dFdx(vec3(uv,z));
    vec3 duvdist_dy = dFdy(vec3(uv,z));
    
    dz_duv.x = duvdist_dy.y * duvdist_dx.z;
    dz_duv.x -= duvdist_dx.y * duvdist_dy.z;
    
    dz_duv.y = duvdist_dx.x * duvdist_dy.z;
    dz_duv.y -= duvdist_dy.x * duvdist_dx.z;
    
    float det = (duvdist_dx.x * duvdist_dy.y) - (duvdist_dx.y * duvdist_dy.x);
    dz_duv /= det;
    
    return dz_duv;
}

float borderDepthTexture(sampler2DArray tex, vec3 uv) {
    return /*((uv.x <= 1.0) && (uv.y <= 1.0) &&
            (uv.x >= 0.0) && (uv.y >= 0.0)) ? */texture(tex, uv).x /*: 1.0*/;
}

float biasedZ(float z0, vec2 dz_duv, vec2 offset) {
    return z0 + dot(dz_duv, offset);
}


#define NO_TANGENT

mat3 computeTangentFrame( const in vec4 tangent, const in vec3 normal)
{
    vec3 tangentx, tangenty;
    // Build local referential
#   if defined( NO_TANGENT )
    vec3 upVector = abs(normal.y) < 0.999999 ? vec3(0.0,1.0,0.0) : vec3(0.0,0.0,1.0);
    tangentx = normalize( cross( upVector, normal ) );
    tangenty = cross( normal, tangentx );
    
#   else
    
    vec3 tang = normalize(tangent.xyz);
    vec3 binormal = tangent.w * cross(normal, tang);
    tangentx = normalize(tang - normal*dot(tang, normal)); // local tangent
    tangenty = normalize(binormal  - normal*dot(binormal, normal)  - tang*dot(binormal, tangentx)); // local bitange
#   endif
    return mat3( tangentx, tangenty, normal );
}

// http://the-witness.net/news/2012/02/seamless-cube-map-filtering/
vec3 fix_cube_lookup( vec3 v, float cube_size, float lod ) {
    float M = max(max(abs(v.x), abs(v.y)), abs(v.z));
    float scale = 1.0 - exp2(lod) / cube_size;
    if (abs(v.x) != M) v.x *= scale;
    if (abs(v.y) != M) v.y *= scale;
    if (abs(v.z) != M) v.z *= scale;
    return v;
}


#ifdef DEBUG

vec3 shCoefs[9];

void createCoef() {
    
    // vec3(  1.0/(2.0*sqrt(PI) ) ),
    
    // vec3( -( sqrt(3.0/PI)*0.5 * y ) ),
    // vec3( ( sqrt(3.0/PI)*0.5 * z ) ),
    // vec3( -( sqrt(3.0/PI)*0.5 * x ) ),
    
    // vec3( ( sqrt(15.0/PI)*0.5 * x * y ) ),
    // vec3( -( sqrt(15.0/PI)*0.5 * y * z ) ),
    // vec3( ( sqrt(5.0/PI)* 0.25 * ( 3.0*z*z - 1.0) ) ),
    // vec3( -( sqrt(15.0/PI)* 0.5 * x *z ) ),
    // vec3( ( sqrt(15.0/PI) * 0.25 * (x*x - y*y )) ),
    
    shCoefs[0] = vec3(  1.0/(2.0*sqrt(PI) ) );
    
    shCoefs[1] = vec3( -( sqrt(3.0/PI)*0.5 ) );
    shCoefs[2] = -shCoefs[1];
    shCoefs[3] = shCoefs[1];
    
    shCoefs[4] = vec3( sqrt(15.0/PI)*0.5 );
    shCoefs[5] = -shCoefs[4];
    shCoefs[6] = vec3( sqrt(5.0/PI)* 0.25 );
    shCoefs[7] = shCoefs[5];
    shCoefs[8] = vec3( sqrt(15.0/PI) * 0.25 );
    
}

vec3 sphericalHarmonics( const in vec3 normal )
{
    float x = normal.x;
    float y = normal.y;
    float z = normal.z;
    
    createCoef();
    vec3 result = (
                   shCoefs[0] * uSph[0] +
                   
                   shCoefs[1] * uSph[1] * y +
                   shCoefs[2] * uSph[2] * z +
                   shCoefs[3] * uSph[3] * x +
                   
                   shCoefs[4] * uSph[4] * y * x +
                   shCoefs[5] * uSph[5] * y * z +
                   shCoefs[6] * uSph[6] * (3.0 * z * z - 1.0) +
                   shCoefs[7] * uSph[7] * (z * x) +
                   shCoefs[8] * uSph[8] * (x*x - y*y)
                   );
}

#else
// expect shCoefs uniform
// https://github.com/cedricpinson/envtools/blob/master/Cubemap.cpp#L523
vec3 sphericalHarmonics( const vec3 sph[9], const in vec3 normal )
{
    float x = normal.x;
    float y = normal.y;
    float z = normal.z;
    
    vec3 result = (
                   sph[0] +
                   
                   sph[1] * y +
                   sph[2] * z +
                   sph[3] * x +
                   
                   sph[4] * y * x +
                   sph[5] * y * z +
                   sph[6] * (3.0 * z * z - 1.0) +
                   sph[7] * (z * x) +
                   sph[8] * (x*x - y*y)
                   );
    
    return max(result, vec3(0.0));
}

#endif