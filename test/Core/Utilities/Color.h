//
//  Color.h
//  Nanocat
//
//  Created by Kawaii Neko on 1/20/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef Color_h
#define Color_h

namespace Neko {
    
    /// Color gradient.
    struct SColorGradient {
    public:
        
        /**
         *  Create new color gradient.
         */
        SColorGradient( const Vec3 color1, const Vec3 color2 ) : firstColor(color1), secondColor(color2) {
            
        }
        
        Vec3   firstColor;
        Vec3   secondColor;
        
        /**
         *  Evaluate color in time.
         */
        inline Vec3 Evaluate( const float time ) {
            Vec3 result;
            
            result.x = firstColor.x * time + secondColor.x * (1.0f - time);
            result.y = firstColor.y * time + secondColor.y * (1.0f - time);
            result.z = firstColor.z * time + secondColor.z * (1.0f - time);
            
            return result;
        }
    };
}

#endif /* Color_h */
