//https://www.shadertoy.com/view/3sXyW2

int PrintInt( in vec2 uv, in int value, const int maxDigits );

#define bits constants.bits

//128 bytes, make bits[32] as push_constant
/*
uint bits[32]=uint[32](0x77777777u,0x88888888u,0x77777777u,0x88888888u,
                       0x77777777u,0x88888888u,0x77777777u,0x88888888u,
                       0x77777777u,0x88888888u,0x77777777u,0x88888888u,
                       0x77777777u,0x88888888u,0x77777777u,0x88888888u,
                       0x77777777u,0x88888888u,0x77777777u,0x88888888u,
                       0x77777777u,0x88888888u,0x77777777u,0x88888888u,
                       0x77777777u,0x88888888u,0x77777777u,0x88888888u,
                       0x77777777u,0x88888888u,0x77777777u,0x88888888u
                      );
*/
uint decodeval16(uint varz, int y) {
    switch(y){
        case 0:return uint((varz>>24)&0xffu);
        case 1:return uint((varz>>16)&0xffu);
        case 2:return uint((varz>>8)&0xffu);
        case 3:return uint((varz>>0)&0xffu);
    }
    return 0u;
}

float main2(in vec2 fragCoord)
{
    fragCoord=fragCoord/iResolution.xy;
    fragCoord=floor(vec2(256.,3.)*fragCoord)+0.5;
    ivec2 ipx=ivec2(fragCoord);
    ipx.y=int(vec2(256.,3.).y)-(ipx.y+1);
    uint ival=decodeval16(bits[clamp(ipx.x/8,0,31)],(ipx.x/2)%4);
    int map[8];
    for (int i=7; i >= 0; i--, ival /= 2u) {
        map[clamp(i,0,7)]=int(ival % 2u);
    }
    return float(map[clamp(1+ipx.y+(ipx.x%2)*4,0,7)]);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    fragColor=vec4(0.);
    
    fragCoord=clamp(fragCoord.xy,vec2(0.),iResolution.xy);
    fragCoord.y=iResolution.y-fragCoord.y;
    
    vec2 res=iResolution.xy/iResolution.y;
    vec2 uv=fragCoord/iResolution.y-0.5*res;
    vec2 ouv=uv;
    int d=0;
    
    int r=0;
    vec2 tuv=(ouv+0.5)*vec2(8.,4.);
    int idx=int(tuv.x)+8*int(tuv.y);
    if((tuv.x>0.)&&(idx>=8*int(tuv.y))&&(idx<8+8*int(tuv.y))){
        tuv=fract(tuv)-0.5;
        r+=PrintInt((tuv+vec2(0.,0.5))*10.,idx,2);
        
        vec2 tuv2=tuv;
        tuv2.x+=0.25;tuv2.y+=-0.05;
        vec2 tuv3=tuv2;
        for(int i=0;i<4;i++){
            uint ivalx=decodeval16(bits[clamp(idx,0,31)],i);
            for (int j=7; j >= 0; j--, ivalx /= 2u) {
                d+=PrintInt(tuv2*15.,int(ivalx % 2u),1);
                tuv2.x+=-0.005*15.;
            }
            tuv2.x=tuv3.x;
            tuv2.y+=0.005*15.;
        }
        
    }
    r=min(r,1);
    d=min(d,1);
    fragColor.rg+=float(r);
    fragColor.rgb+=float(d);
    fragColor.r+=main2(fragCoord);
    fragColor.a=1.;
}

//https://www.shadertoy.com/view/ldsyz4
// The MIT License
// Copyright © 2017 Inigo Quilez
// Digit data by P_Malin (https://www.shadertoy.com/view/4sf3RN)
const int[] font = int[](0x75557, 0x22222, 0x74717, 0x74747, 0x11574, 0x71747, 0x71757, 0x74444, 0x75757, 0x75747);
const int[] powers = int[](1, 10, 100, 1000, 10000, 100000, 1000000);

int PrintInt( in vec2 uv, in int value, const int maxDigits )
{
    if( abs(uv.y-0.5)<0.5 )
    {
        int iu = int(floor(uv.x));
        if( iu>=0 && iu<maxDigits )
        {
            int n = (value/powers[clamp(maxDigits-iu-1,0,6)]) % 10;
            uv.x = fract(uv.x);//(uv.x-float(iu)); 
            ivec2 p = ivec2(floor(uv*vec2(4.0,5.0)));
            return (font[clamp(n,0,9)] >> (p.x+p.y*4)) & 1;
        }
    }
    return 0;
}
