
 int int32xfp16(int a,int w){  //a:int32 from int8 , w: fp16 from dequal
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

  if(((w>>10)&0x1f)==0) w_frac = (w & 0x3ff);   
  else                  w_frac = (0x1<<10) | (w & 0x3ff);
  w_exp = (w>>10) & 0x1f;
  res_pos_int = 30 - w_exp;
  if(w_exp==0 && w_frac!=0){
    tmp = w_frac;
    while(((tmp>>9)&0x1)==0){
      tmp = tmp<<1;
      res_pos_int++;
    }
    w_frac = tmp<<1; 
  }
  if(((a>>31)&0x1)==1) a_abs = -a;
  else                 a_abs = a;
  
  res_mul  =  (long long)a_abs * (long long)w_frac;
  res_mul = res_mul << 5;  

  pos_max  = res_pos_int + 15;
  pos_min  = res_pos_int - 15;

  res_sign  =   ((a>>31)&0x1)^((w>>15)&0x1);
  if((res_mul>>(pos_max+1))!=0){
       res_exp   =   31;      //inf
       res_frac  =   0;     
  }
  else {
    pos = pos_max;
    res_exp = 30;
    while(((res_mul>>pos)&0x1)==0 && (pos>pos_min) && res_exp>0){
      pos--;
      res_exp--;
    }
    if(res_exp==0){
        if(((res_mul>>(pos-10)) & 0x7ff)==0x7ff){
          res_exp   = 1;
          res_frac  = 0;
        }
        else {
          res_frac = ((res_mul>>(pos-9)) & 0x3ff) + ((res_mul>>(pos-10))&0x1);
        }
    }
    else{
        if(((res_mul>>(pos-11)) & 0x7ff)==0x7ff){
          res_exp   += 1;
          res_frac  = 0;
        }
        else{
           res_frac = ((res_mul>>(pos-10)) & 0x3ff) + ((res_mul>>(pos-11))&0x1) ;
        }
    }  
  }
  
  if(w_exp==31 && (w&0x3ff)!=0)                      cal_res  =  0x7c01;  //nan
  else if(w_exp==31 && (w&0x3ff)==0 && a==0)         cal_res  =  0x7c01;  //inf*0=nan
  else if(w_exp==31 && (w&0x3ff)==0 && res_sign==0)  cal_res  =  0x7c00;  //inf
  else if(w_exp==31 && (w&0x3ff)==0 && res_sign==1)  cal_res  =  0xfc00;  //inf
  else if((a==0 || (w&0x7fff)==0) && res_sign==0)    cal_res  =  0x0;     //0*0       
  else if((a==0 || (w&0x7fff)==0) && res_sign==1)    cal_res  =  0x8000;  //0*0       
  else                                               cal_res  =  ((res_sign&0x1)<<15) | ((res_exp&0x1f)<<10) | (res_frac&0x3ff); 
  
  //printf("int32xfp16 a:0x%x w:0x%x result:0x%x.\n", a, w, cal_res);
  return  cal_res;
}
