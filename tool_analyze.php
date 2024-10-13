<?php

$we["255.255.255.255"]=32; $we["255.255.255.254"]=31; $we["255.255.255.252"]=30; $we["255.255.255.248"]=29;
$we["255.255.255.240"]=28; $we["255.255.255.224"]=27; $we["255.255.255.192"]=26; $we["255.255.255.128"]=25;
$we["255.255.255.0"]=24; $we["255.255.254.0"]=23; $we["255.255.252.0"]=22; $we["255.255.248.0"]=21;
$we["255.255.240.0"]=20; $we["255.255.224.0"]=19; $we["255.255.192.0"]=18; $we["255.255.128.0"]=17;
$we["255.255.0.0"]=16; $we["255.254.0.0"]=15; $we["255.252.0.0"]=14; $we["255.248.0.0"]=13; $we["255.240.0.0"]=12;

function mys($a){
  return substr($a,1,strlen($a)-2);
}

$aux=explode("\n",file_get_contents("https://docs.google.com/spreadsheets/d/1DTngQUDqQgcYhA4S1iOW3jGuj-nOO-98opbXeUC-ffA/gviz/tq?tq=select%20A%2CB%2CE%2CF%2CG&tqx=out:csv&gid=644800101"));
for($i=1;;$i++){
  if(!isset($aux[$i]))break;
  $aa=explode(",",$aux[$i]);
  $idstart[$i]=mys($aa[0]);
  $idend[$i]=mys($aa[1]);
  $ip[$i]=mys($aa[2]);
  $pal[$i]=mys($aa[3]);
  $ssid[$i]=mys($aa[4]);
  $used[$i]=0;
}

$fp=fopen("qq","r");
for($v=0;;){
  $aux=trim(fgets($fp,300));
  if(feof($fp))break;
  if(strlen($aux)<5)continue;
  if($aux[0]=="#"&&$aux[1]=="#")$mpal=substr($aux,3);
  else if($aux[0]=="#")$mssid=substr($aux,2);
  else if(substr($aux,0,6)=="subnet"){
    $aa=explode(" ",$aux);
    $vv=explode(".",$aa[1]);
    $a1=$vv[1]*256+$vv[2];
    $mwe=$we[$aa[3]];
    $a2=$a1+2**(24-$mwe)-1;
    for($j=1;$j<$i;$j++){
      if($a1==$idstart[$j] && $a2==$idend[$j] && $mpal==$pal[$j] && $mssid==$ssid[$j]){
        $used[$j]=1;
        break;
      }
    }
    if($j>=$i)$out[$v++]="$a1,$a2,$aa[1]/$mwe,$mpal,$mssid";
  }
}
fclose($fp);

if($v==0)echo "All is ok\n";
else {
  sort($out);
  print_r($out);
}

echo "UnUsed\n";
for($j=1;$j<$i;$j++)if($used[$j]==0)echo $idstart[$j].",".$idend[$j]."\n";

?>
