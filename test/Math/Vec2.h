//
//  Vec2.h
//  Neko engine
//
//  Created by Kawaii Neko on 4/10/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef Vec2_h
#define Vec2_h

#include "GameMath.h"

namespace Neko {
    
    class ncMatrix4;
    template <class t> class vec2_template;
    
    typedef vec2_template<float> Vec2;
    typedef vec2_template<int> Vec2i;
    typedef vec2_template<uint8_t> Vec2i8;
    
    
    /**
     *  2-dimensional vector.
     */
    template <class t> class vec2_template {
    public:
        t x;
        t y;
        
        vec2_template() : x(0), y(0) { }
        vec2_template( t v ) : x( v ), y( v ) { }
        vec2_template( t x1, t y1 ) : x( x1 ), y( y1 ) { }
        
        vec2_template<t> operator - (const float& v) const
        {
            vec2_template<t> vResult;
            vResult.x = x - v;
            vResult.y = y - v;
            
            return vResult;
        }
        
        vec2_template<t> operator - (const vec2_template<t>& v) const
        {
            vec2_template<t> vResult;
            vResult.x = x - v.x;
            vResult.y = y - v.y;
            
            return vResult;
        }
        
        vec2_template<t> operator + (const vec2_template<t>& v) const
        {
            vec2_template<t> vResult;
            vResult.x = x + v.x;
            vResult.y = y + v.y;
            
            return vResult;
        }
        
        vec2_template<t> operator / (const vec2_template<t>& v) const
        {
            vec2_template<t> vResult;
            vResult.x = x / v.x;
            vResult.y = y / v.y;
            
            return vResult;
        }
        
        
        /**
         *  Make all vector values to zero.
         */
        inline void                 MakeNull() { x = y = 0.0f; }
        
        float operator * ( const vec2_template<t>& v ) {
            return this->x * v.x + this->y * v.y;
        }
        
        vec2_template<t> operator               - (const t v) const;
        
        vec2_template<t> operator + (const t v) const
        {
            vec2_template<t> vResult;
            vResult.x = x + v;
            vResult.y = y + v;
            
            return vResult;
        }
        
        inline vec2_template<t> operator / (const t v) const
        {
            vec2_template<t> vResult;
            vResult.x = x / v;
            vResult.y = y / v;
            
            return vResult;
        }
        
        inline vec2_template<t> operator * (const t v) const
        {
            vec2_template<t> vResult;
            vResult.x = x * v;
            vResult.y = y * v;
            
            return vResult;
        }
        
        inline vec2_template<t> operator*( const vec2_template<t>& v ) const
        {
            vec2_template<t> vResult;
            vResult.x = x * v.x;
            vResult.y = y * v.y;
            
            return vResult;
        }
        
        
        inline const float Length( void ) const
        {
            return( (float)nkMath::FastSqrty( x * x + y * y ) );
        }
        
        inline vec2_template Normalize()
        {
            float l = this->Length();
            return vec2_template( this->x / l, this->y / l );
        }
        
        /**
         *  Angle between two vectors.
         */
        inline static float Angle( const vec2_template<t> & v1, const vec2_template<t> & v2 )
        {
            // cross product
            t y = (v1.x * v2.y) - (v2.x * v1.y);
            
            // dot product
            t x = (v1.x * v2.x) + (v1.y * v2.y);
            
            return (t)atan2f(y, x);
        }
        
    };
    
}

#endif /* Vec2_h */
