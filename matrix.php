<?php

include "data.php";
date_default_timezone_set("Europe/Rome");

$ip=$_GET["ip"];
@$ser=$_GET["ser"];
if($ser=="")$ser="0000";
$des="tmp/files/$ip.des";
$ff="tmp/files/$ip.ff";
$bin="tmp/files/$ip.bin";
$conn=oci_connect($p1,$p2,$p3);

$aux=explode(".",$ip);
if($aux[0]=="10"){
  $id=$aux[1]*256+$aux[2];
  $query=oci_parse($conn,"select istat from idistat where '$id'>=idstart and '$id'<=idend");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $istat=$row[0];
  oci_free_statement($query);
}
else if($aux[0]=="WEB")$istat=$aux[3];
else $istat="37006";

$query=oci_parse($conn,"select sovra from idistat where istat='$istat'");
oci_execute($query);
$row=oci_fetch_row($query);
@$sovra=$row[0];
oci_free_statement($query);

$query=oci_parse($conn,"select ente from istatente where istat='$istat'");
oci_execute($query);
$row=oci_fetch_row($query);
@$ente=$row[0];
oci_free_statement($query);

function mysplit($x,$len){
  $o=0;
  for($l=0;;){
    $s=strpos($x," ",$l);
    if($s==null)$ss=strlen($x);
    else $ss=$s;
    if(strlen($oo[$o])+$ss-$l+1>$len)$o++;
    if(strlen($oo[$o])==0)$oo[$o]=substr($x,$l,$ss-$l);
    else $oo[$o].=" ".substr($x,$l,$ss-$l);
    if($s==null)return $oo;
    $l=$ss+1;
  }
}

function show1($table,$par,$title,$istat,$sovra,$des,$ff,$bin,$time,$conn,$privacy){
  $query=oci_parse($conn,"select $par from $table where istat='$istat'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=$row[0];
  oci_free_statement($query);
  $query=oci_parse($conn,"select $par from $table where istat='00008'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$reraux=$row[0];
  oci_free_statement($query);
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 0 FFFFFFFF 00000000 2 %s\n",$title);
  if($sovra<>""){
    $query=oci_parse($conn,"select $par from $table where istat='$sovra'");
    oci_execute($query);
    $row=oci_fetch_row($query);
    @$sovraaux=$row[0];
    oci_free_statement($query);
    fprintf($fp,"1 -2 29 FF0000FF 00000000 1 Unione\n");
    fprintf($fp,"1 -2 38 FF00FFFF 00000000 2 %s\n",($sovraaux<3 & $privacy)?"*":number_format($sovraaux,0,",","."));
    $delta=0;
  }
  else $delta=7;
  fprintf($fp,"1 -2 %02d FF0000FF 00000000 1 Comune\n",10+$delta);
  fprintf($fp,"1 -2 %02d FF00FFFF 00000000 2 %s\n",19+$delta,($aux<3 & $privacy)?"*":number_format($aux,0,",","."));
  fprintf($fp,"1 -2 %02d FF0000FF 00000000 1 Regione\n",48-$delta);
  fprintf($fp,"1 -2 %02d FF00FFFF 00000000 2 %s\n",57-$delta,($reraux<3 & $privacy)?"*":number_format($reraux,0,",","."));
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  return;
}

function show2($base,$tot,$ip,$bin,$time,$conn){
  $query=oci_parse($conn,"update mysession set c1=c1+1 where id='$ip'");
  oci_execute($query);
  oci_free_statement($query);
  $query=oci_parse($conn,"select c1 from mysession where id='$ip'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $vf=$base + $row[0] % $tot;
  oci_free_statement($query);
  $name=sprintf("tmp/image/%04d.ff",$vf);
  shell_exec("tmp/convert3 $name $time $bin");
}

function show3($table,$par,$istat,$conn){
  $query=oci_parse($conn,"select $par from $table where istat='$istat'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=$row[0];
  oci_free_statement($query);
  return $aux;
}

function myrandom($conn,$ip,$tot){
  $query=oci_parse($conn,"select c1,c2 from mysession where id='$ip'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $c1=$row[0];
  $c2=$row[1];
  oci_free_statement($query);
  $query=oci_parse($conn,"select m,num from rnd1 where p=$tot");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $m=$row[0];
  $num=$row[1];
  oci_free_statement($query);
  $id=floor($c1 / $m) % $num;
  $query=oci_parse($conn,"select a,c from rnd2 where m=$m and id=$id");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $a=$row[0];
  $c=$row[1];
  oci_free_statement($query);
  $next=($a * $c2 + $c) % $m;
  $query=oci_parse($conn,"update mysession set c1=c1+1,c2=$next where id='$ip'");
  oci_execute($query);
  oci_free_statement($query);
  return (($next<$tot) ? $next : rand(0,$tot-1));
}

$query=oci_parse($conn,"select count(*) from mysession where id='$ip'");
oci_execute($query);
$row=oci_fetch_row($query);
@$myexist=$row[0];
oci_free_statement($query);
if(!$myexist){
  $query=oci_parse($conn,"insert into mysession (id,ser,iter,c1,c2) values ('$ip','$ser',0,0,0)");
  oci_execute($query);
  oci_free_statement($query);
}
$query=oci_parse($conn,"update mysession set iter=iter+1 where id='$ip'");
oci_execute($query);
oci_free_statement($query);
$query=oci_parse($conn,"update mysession set ser='$ser' where id='$ip'");
oci_execute($query);
oci_free_statement($query);
$query=oci_parse($conn,"select iter from mysession where id='$ip'");
oci_execute($query);
$row=oci_fetch_row($query);
$iter=$row[0];
oci_free_statement($query);

$query=oci_parse($conn,"select seq from sel where ser='$ser'");
oci_execute($query);
$row=oci_fetch_row($query);
@$seq=$row[0];
if($seq=="")$seq="0000";
oci_free_statement($query);
$query=oci_parse($conn,"select count(*) from seq where seq='$seq'");
oci_execute($query);
$row=oci_fetch_row($query);
@$nseq=$row[0];
if($nseq=="")$nseq=1;
oci_free_statement($query);
$mid=$iter % $nseq;
$query=oci_parse($conn,"select screen,time from seq where seq='$seq' and id=$mid");
oci_execute($query);
$row=oci_fetch_row($query);
@$screen=$row[0];
if($screen=="")$screen="0000";
@$time=$row[1];
if($time=="")$time=1;
oci_free_statement($query);

switch($screen){ 
  case "0001": show1("uiftth","uiftth","FTTH bianche",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0002": show1("areeaai","areeaai","AAI Aree",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0003": show1("aziendeaai","aziendeaai","AAI Aziende",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0004": show1("scuole","scuole","Scuole 1G",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0005": show1("pal","pal","PAL rete",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0006": show1("man","man","MAN rete",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0007": show1("apwifi","apwifi","Punti WiFi",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0008": show1("userwifi","userwifi","Utenti WiFi",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0009": show1("attivifse","attivi","Attivi FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0010": show1("accessifse","accessi","Accessi FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0011": show1("scaricatifse","scaricati","Scaricati FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0012": show1("attivazionilepidaid","attivazioni","Attivazioni ID",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0013": show1("accessilepidaid","accessi","Accessi ID",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0014": show1("sportellilepidaid","sportelli","Sportelli ID",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;

  case "8001": shell_exec("tmp/convert3 tmp/image/L001.ff $time $bin"); break;
  case "8002": shell_exec("tmp/convert3 tmp/image/L002.ff $time $bin"); break;
  case "8003": shell_exec("tmp/convert3 tmp/image/L003.ff $time $bin"); break;
  case "8004": shell_exec("tmp/convert3 tmp/image/L004.ff $time $bin"); break;
  case "8005": shell_exec("tmp/convert3 tmp/image/L005.ff $time $bin"); break;
  case "8006": shell_exec("tmp/convert3 tmp/image/L006.ff $time $bin"); break;

  case "9001": show2(1,76,$ip,$bin,$time,$conn); break;
  case "9002": show2(1001,28,$ip,$bin,$time,$conn); break;
  case "9003": show2(2001,580,$ip,$bin,$time,$conn); break;
 
  case "9103":
  $vf=2001+myrandom($conn,$ip,580);
  $name=sprintf("tmp/image/%04d.ff",$vf);
  shell_exec("tmp/convert3 $ff $time $bin");
  break;
  
  case "9203":
  $vf=2001+myrandom($conn,$ip,580);
  $ih=3.0; $Ih=18.0; $im=3.0; $Im=29.0;
  $llt=localtime(time(),true);
  $hh=$llt["tm_hour"];
  $mm=$llt["tm_min"];
  if($hh>11)$hh-=12;
  $hr=(90.0-30.0*($hh+$mm/60.0))*2.0*M_PI/360.0;
  $mr=(90.0-6.0*$mm)*2.0*M_PI/360.0;
  $fp=fopen($des,"w");
  fprintf($fp,"5 %04d FF\n",$vf);
  fprintf($fp,"9 %d %d %d %d V1\n",31.0+$ih*cos($hr),63.0-31.0-$ih*sin($hr),31.0+$Ih*cos($hr),63.0-31.0-$Ih*sin($hr));
  fprintf($fp,"3 %d %d %d %d V1iFF\n",31.0+$ih*cos($hr),63.0-31.0-$ih*sin($hr),31.0+$Ih*cos($hr),63.0-31.0-$Ih*sin($hr));
  fprintf($fp,"9 %d %d %d %d V2\n",31.0+$im*cos($mr),63.0-31.0-$im*sin($mr),31.0+$Im*cos($mr),63.0-31.0-$Im*sin($mr));
  fprintf($fp,"3 %d %d %d %d V2iFF\n",31.0+$im*cos($mr),63.0-31.0-$im*sin($mr),31.0+$Im*cos($mr),63.0-31.0-$Im*sin($mr));
  if(abs(cos($hr))>0.7){
    fprintf($fp,"9 %d %d %d %d V3\n",31.0+$ih*cos($hr),63.0-31.0-$ih*sin($hr)+1,31.0+$Ih*cos($hr),63.0-31.0-$Ih*sin($hr))+1;
    fprintf($fp,"3 %d %d %d %d V3iFF\n",31.0+$ih*cos($hr),63.0-31.0-$ih*sin($hr)+1,31.0+$Ih*cos($hr),63.0-31.0-$Ih*sin($hr))+1;
    fprintf($fp,"9 %d %d %d %d V4\n",31.0+$ih*cos($hr),63.0-31.0-$ih*sin($hr)-1,31.0+$Ih*cos($hr),63.0-31.0-$Ih*sin($hr)-1);
    fprintf($fp,"3 %d %d %d %d V4iFF\n",31.0+$ih*cos($hr),63.0-31.0-$ih*sin($hr)-1,31.0+$Ih*cos($hr),63.0-31.0-$Ih*sin($hr)-1);
  }
  else {
    fprintf($fp,"9 %d %d %d %d V3\n",31.0+$ih*cos($hr)+1,63.0-31.0-$ih*sin($hr),31.0+$Ih*cos($hr)+1,63.0-31.0-$Ih*sin($hr));
    fprintf($fp,"3 %d %d %d %d V3iFF\n",31.0+$ih*cos($hr)+1,63.0-31.0-$ih*sin($hr),31.0+$Ih*cos($hr)+1,63.0-31.0-$Ih*sin($hr));
    fprintf($fp,"9 %d %d %d %d V4\n",31.0+$ih*cos($hr)-1,63.0-31.0-$ih*sin($hr),31.0+$Ih*cos($hr)-1,63.0-31.0-$Ih*sin($hr));
    fprintf($fp,"3 %d %d %d %d V4iFF\n",31.0+$ih*cos($hr)-1,63.0-31.0-$ih*sin($hr),31.0+$Ih*cos($hr)-1,63.0-31.0-$Ih*sin($hr));   
  }
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  break;

  case "7001":
  $aux=mysplit($ente,12);
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 0 FF0000FF 00000000 3 %sZ\n",date("d.m.y H:i"));
  fprintf($fp,"1 -2 6 FFFFFFFF 00000000 3 %s %s\n",$ip,$ser);
  fprintf($fp,"1 0 12 FFFF00FF 00000000 3 Istat\n");
  fprintf($fp,"1 0 18 0000FFFF 00000000 3 %s\n",$istat);
  if($sovra<>""){
    fprintf($fp,"1 -1 12 00FFFFFF 00000000 3 Unione\n");
    fprintf($fp,"1 -1 18 FF0000FF 00000000 3 %s\n",$sovra);
  }
  fprintf($fp,"1 -2 32 FFFFFFFF 00000000 2 %s\n",$aux[0]);
  fprintf($fp,"1 -2 40 FFFFFFFF 00000000 2 %s\n",@$aux[1]);
  fprintf($fp,"1 -2 48 FFFFFFFF 00000000 2 %s\n",@$aux[2]);
  fprintf($fp,"1 -2 56 FFFFFFFF 00000000 2 %s\n",@$aux[3]);
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  break;

  case "4001":
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 5 FFFFFFFF 00000000 1 Utenti\n");
  fprintf($fp,"1 -2 15 FFFFFFFF 00000000 1 attivi\n");
  fprintf($fp,"1 -2 25 00FF00FF 00000000 1 FSE\n");
  $aux=show3("attivifse","attivi","00008",$conn);
  fprintf($fp,"1 -2 46 00FF00FF 00000000 2 %s\n",($aux<3)?"*":number_format($aux,0,",","."));
  fprintf($fp,"2 0 61 63 63 00FF00FF\n");
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  break;

  case "4002":
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 5 FFFFFFFF 00000000 1 Accessi\n");
  fprintf($fp,"1 -2 18 00FF00FF 00000000 1 FSE\n");
  $aux=show3("accessifse","accessi","00008",$conn);
  fprintf($fp,"1 -2 46 00FF00FF 00000000 2 %s\n",($aux<3)?"*":number_format($aux,0,",","."));
  fprintf($fp,"2 0 61 63 63 00FF00FF\n");
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  break;

  case "4003":
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 5 FFFFFFFF 00000000 1 Documenti\n");
  fprintf($fp,"1 -2 15 FFFFFFFF 00000000 1 scaricati\n");
  fprintf($fp,"1 -2 25 00FF00FF 00000000 1 FSE\n");
  $aux=show3("scaricatifse","scaricati","00008",$conn);
  fprintf($fp,"1 -2 46 00FF00FF 00000000 2 %s\n",($aux<3)?"*":number_format($aux,0,",","."));
  fprintf($fp,"2 0 61 63 63 00FF00FF\n");
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  break;

  case "4004":
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 5 FFFFFFFF 00000000 1 Cittadini\n");
  fprintf($fp,"1 -2 15 FFFFFFFF 00000000 1 con\n");
  fprintf($fp,"1 -2 25 FF8000FF 00000000 1 LepidaID\n");
  $aux=show3("attivazionilepidaid","attivazioni","00008",$conn);
  fprintf($fp,"1 -2 46 FF8000FF 00000000 2 %s\n",($aux<3)?"*":number_format($aux,0,",","."));
  fprintf($fp,"2 0 61 63 63 FF8000FF\n");
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  break;

  case "4005":
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 5 FFFFFFFF 00000000 1 Accessi\n");
  fprintf($fp,"1 -2 15 FF8000FF 00000000 1 LepidaID\n");
  $aux=show3("accessilepidaid","accessi","00008",$conn);
  fprintf($fp,"1 -2 46 FF8000FF 00000000 2 %s\n",($aux<3)?"*":number_format($aux,0,",","."));
  fprintf($fp,"2 0 61 63 63 FF8000FF\n");
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  break;

  case "4006":
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 5 FFFFFFFF 00000000 1 Sportelli\n");
  fprintf($fp,"1 -2 15 FF8000FF 00000000 1 LepidaID\n");
  $aux=show3("sportellilepidaid","sportelli","00008",$conn);
  fprintf($fp,"1 -2 46 FF8000FF 00000000 2 %s\n",($aux<3)?"*":number_format($aux,0,",","."));
  fprintf($fp,"2 0 61 63 63 FF8000FF\n");
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  break;

  case "4007":
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 5 FFFFFFFF 00000000 1 Punti\n");
  fprintf($fp,"1 -2 15 FFFFFFFF 00000000 1 E-R\n");
  fprintf($fp,"1 -2 25 008080FF 00000000 1 WiFi\n");
  $aux=show3("apwifi","apwifi","00008",$conn);
  fprintf($fp,"1 -2 46 008080FF 00000000 2 %s\n",($aux<3)?"*":number_format($aux,0,",","."));
  fprintf($fp,"2 0 61 63 63 008080FF\n");
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  break;

  case "4008":
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 5 FFFFFFFF 00000000 1 Utenti\n");
  fprintf($fp,"1 -2 15 FFFFFFFF 00000000 1 E-R\n");
  fprintf($fp,"1 -2 25 008080FF 00000000 1 WiFi\n");
  $aux=show3("userwifi","userwifi","00008",$conn);
  fprintf($fp,"1 -2 46 008080FF 00000000 2 %s\n",($aux<3)?"*":number_format($aux,0,",","."));
  fprintf($fp,"2 0 61 63 63 008080FF\n");
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  break;

  default:
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 0 FF0000FF 00000000 2 ser %s\n",$ser);
  fprintf($fp,"1 -2 10 00FF00FF 00000000 2 seq %s\n",$seq);
  fprintf($fp,"1 -2 20 0000FFFF 00000000 2 nseq %d\n",$nseq);
  fprintf($fp,"1 -2 30 FFFF00FF 00000000 2 mid %d\n",$mid);
  fprintf($fp,"1 -2 40 FF00FFFF 00000000 2 screen %s\n",$screen);
  fprintf($fp,"1 -2 50 00FFFFFF 00000000 2 time %d\n",$time);
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff 10 $bin");
  break;
}

$len=filesize($bin);
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile($bin);

oci_close($conn);

?>
