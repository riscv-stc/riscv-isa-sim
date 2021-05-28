#ifndef _INT32_X_BF16
#define _INT32_X_BF16
 
 int int32xbf16(int a,int w){  //a:int32 from int8 , w: fp16 from dequal
  int w_frac;
  int w_exp;
  int res_pos_int; //result integer start pos 
  unsigned int a_abs;    //|a|
  long long res_mul;  //a * w_frac
  int tmp;  
  int pos;
  int pos_max;
  int pos_min;
  int res_sign;       //result sign
  int res_exp;        //result exp
  int res_frac;       //result frac
  int cal_res;        //result

  if(((w>>7)&0xff)==0)  w_frac = (w & 0x7f);   
  else                  w_frac = (0x1<<7) | (w & 0x7f);
  w_exp = (w>>7) & 0xff;
  res_pos_int = w_exp;
  if(w_exp==0 && w_frac!=0){
    tmp = w_frac;
    while(((tmp>>6)&0x1)==0){
      tmp = tmp<<1;
      res_pos_int--;
    }
    w_frac = tmp<<1; 
  }
  if(((a>>31)&0x1)==1) a_abs = -a;
  else                 a_abs = a;
//printf("debug0 a:%x, w:%x, a_abs:%x,w_frac:%x \n",a,w,a_abs,w_frac);
  
  res_mul  =  (long long)a_abs * (long long)w_frac;
  res_mul = res_mul << 8;  

  pos_max  = 47;
  pos_min  = 15;

//printf("debug1 res_mul:%llx, res_pos_int:%x \n",res_mul,res_pos_int);
  res_sign  =   ((a>>31)&0x1)^((w>>15)&0x1);

  pos     = pos_max;
  res_exp = res_pos_int + 32;
  while(((res_mul>>pos)&0x1)==0 && (pos>pos_min) && res_exp>0){
      pos--;
      res_exp--;
  }
//printf("debug2 res_exp:%x, pos:%x \n",res_exp,pos);
  if(res_exp>=255){
       res_exp   =   255;      //inf
       res_frac  =   0;     
  }
  else if(res_exp==0){
        if(((res_mul>>(pos-7)) & 0xff)==0xff){
          res_exp   = 1;
          res_frac  = 0;
        }
        else {
          res_frac = ((res_mul>>(pos-6)) & 0x7f) + ((res_mul>>(pos-7))&0x1);
        }
  }
  else{
        if(((res_mul>>(pos-8)) & 0xff)==0xff){
          res_exp   += 1;
          res_frac  = 0;
        }
        else{
           res_frac = ((res_mul>>(pos-7)) & 0x7f) + ((res_mul>>(pos-8))&0x1) ;
        }
  }  
//printf("debug3 res_exp:%x, res_frac:%x \n",res_exp,res_frac);
  
  if(w_exp==255 && (w&0x7f)!=0)                     cal_res  =  0xffc0;  //nan
  else if(w_exp==255 && (w&0x7f)==0 && a==0)         cal_res  =  0xffc0;  //inf*0=nan
  else if(w_exp==255 && (w&0x7f)==0 && res_sign==0)  cal_res  =  0x7f80;  //inf
  else if(w_exp==255 && (w&0x7f)==0 && res_sign==1)  cal_res  =  0xff80;  //inf
  else if((a==0 || (w&0x7fff)==0) && res_sign==0)    cal_res  =  0x0;     //0*0       
  else if((a==0 || (w&0x7fff)==0) && res_sign==1)    cal_res  =  0x8000;  //0*0       
  else                                               cal_res  =  ((res_sign&0x1)<<15) | ((res_exp&0xff)<<7) | (res_frac&0x7f); 
  
  return  cal_res;
}


#endif