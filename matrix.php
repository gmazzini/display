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
  case "0001": show10("uiftth","uiftth","FTTH bianche",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0002": show10("areeaai","areeaai","AAI Aree",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0003": show10("aziendeaai","aziendeaai","AAI Aziende",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0004": show10("scuole","scuole","Scuole 1G",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0005": show10("pal","pal","PAL rete",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0006": show10("man","man","MAN rete",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0007": show10("apwifi","apwifi","Punti WiFi",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0008": show10("userwifi","userwifi","Utenti WiFi",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0009": show10("attivifse","attivi","Attivi FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0010": show10("accessifse","accessi","Accessi FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0011": show10("scaricatifse","scaricati","Scaricati FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0012": show10("attivazionilepidaid","attivazioni","Attivazioni ID",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0013": show10("accessilepidaid","accessi","Accessi ID",$istat,$sovra,$des,$ff,$bin,$time,$conn,1); break;
  case "0014": show10("sportellilepidaid","sportelli","Sportelli ID",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0015": show10("uifwa","uifwa","FWA bianche",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  case "0016": show10("sensori","sensori","Sensori",$istat,$sovra,$des,$ff,$bin,$time,$conn,0); break;
  
  case "0101": show11("uiftth","uiftth","FTTH bianche",$istat,$sovra,$des,$ff,$bin,$time,$conn,0,0); break;
  case "0102": show11("areeaai","areeaai","AAI Aree",$istat,$sovra,$des,$ff,$bin,$time,$conn,0,2); break;
  case "0103": show11("aziendeaai","aziendeaai","AAI Aziende",$istat,$sovra,$des,$ff,$bin,$time,$conn,0,3); break;
  case "0104": show11("scuole","scuole","Scuole 1G",$istat,$sovra,$des,$ff,$bin,$time,$conn,0,4); break;
  case "0105": show11("pal","pal","PAL rete",$istat,$sovra,$des,$ff,$bin,$time,$conn,0,5); break;
  case "0106": show11("man","man","MAN rete",$istat,$sovra,$des,$ff,$bin,$time,$conn,0,6); break;
  case "0107": show11("apwifi","apwifi","Punti WiFi",$istat,$sovra,$des,$ff,$bin,$time,$conn,0,7); break;
  case "0108": show11("userwifi","userwifi","Utenti WiFi",$istat,$sovra,$des,$ff,$bin,$time,$conn,1,8); break;
  case "0109": show11("attivifse","attivi","Attivi FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn,1,9); break;
  case "0110": show11("accessifse","accessi","Accessi FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn,1,10); break;
  case "0111": show11("scaricatifse","scaricati","Scaricati FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn,1,11); break;
  case "0112": show11("attivazionilepidaid","attivazioni","Attivazioni ID",$istat,$sovra,$des,$ff,$bin,$time,$conn,1,12); break;
  case "0113": show11("accessilepidaid","accessi","Accessi ID",$istat,$sovra,$des,$ff,$bin,$time,$conn,1,13); break;
  case "0114": show11("sportellilepidaid","sportelli","Sportelli ID",$istat,$sovra,$des,$ff,$bin,$time,$conn,0,14); break;
  case "0115": show11("uifwa","uifwa","FWA bianche",$istat,$sovra,$des,$ff,$bin,$time,$conn,0,1); break;
  case "0116": show11("sensori","sensori","Sensori",$istat,$sovra,$des,$ff,$bin,$time,$conn,0,15); break;

  case "8001": shell_exec("tmp/convert3 tmp/image/L001.ff $time $bin"); break;
  case "8002": shell_exec("tmp/convert3 tmp/image/L002.ff $time $bin"); break;
  case "8003": shell_exec("tmp/convert3 tmp/image/L003.ff $time $bin"); break;
  case "8004": shell_exec("tmp/convert3 tmp/image/L004.ff $time $bin"); break;
  case "8005": shell_exec("tmp/convert3 tmp/image/L005.ff $time $bin"); break;
  case "8006": shell_exec("tmp/convert3 tmp/image/L006.ff $time $bin"); break;
  case "8007": shell_exec("tmp/convert3 tmp/image/L007.ff $time $bin"); break;
  case "8008": shell_exec("tmp/convert3 tmp/image/L008.ff $time $bin"); break;
  case "8009": shell_exec("tmp/convert3 tmp/image/L009.ff $time $bin"); break;
  case "8010": shell_exec("tmp/convert3 tmp/image/L010.ff $time $bin"); break;
  case "8011": shell_exec("tmp/convert3 tmp/image/L011.ff $time $bin"); break;
  case "8012": shell_exec("tmp/convert3 tmp/image/L012.ff $time $bin"); break;
  case "8013": shell_exec("tmp/convert3 tmp/image/L013.ff $time $bin"); break;
  case "8014": shell_exec("tmp/convert3 tmp/image/L014.ff $time $bin"); break;
  case "8015": shell_exec("tmp/convert3 tmp/image/L015.ff $time $bin"); break;
  case "8016": shell_exec("tmp/convert3 tmp/image/L016.ff $time $bin"); break;
  case "8017": shell_exec("tmp/convert3 tmp/image/L017.ff $time $bin"); break;
  case "8018": shell_exec("tmp/convert3 tmp/image/L018.ff $time $bin"); break;
  case "8019": shell_exec("tmp/convert3 tmp/image/L019.ff $time $bin"); break;
  case "8020": shell_exec("tmp/convert3 tmp/image/L020.ff $time $bin"); break;
  

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

  case "4001": show4("attivifse","attivi",$des,$ff,$bin,$time,$conn,"Utenti","attivi","FSE","00FF00FF"); break;
  case "4002": show5("accessifse","accessi",$des,$ff,$bin,$time,$conn,"Accessi","FSE","00FF00FF"); break;
  case "4003": show4("scaricatifse","scaricati",$des,$ff,$bin,$time,$conn,"Documenti","scaricati","FSE","00FF00FF"); break;
  case "4004": show4("attivazionilepidaid","attivazioni",$des,$ff,$bin,$time,$conn,"Cittadini","con","LepidaID","FF8000FF"); break;
  case "4005": show5("accessilepidaid","accessi",$des,$ff,$bin,$time,$conn,"Accessi","LepidaID","FF8000FF"); break;
  case "4006": show5("sportellilepidaid","sportelli",$des,$ff,$bin,$time,$conn,"Sportelli","LepidaID","FF8000FF"); break;
  case "4007": show4("apwifi","apwifi",$des,$ff,$bin,$time,$conn,"Punti","E-R","WiFi","008080FF"); break;
  case "4008": show4("userwifi","userwifi",$des,$ff,$bin,$time,$conn,"Utenti","E-R","WiFi","008080FF"); break;
  case "4009": show4("scuole","scuole",$des,$ff,$bin,$time,$conn,"Scuole","connesse","1Gbps","0000FFFF"); break;
  case "4010": show4("uiftth","uiftth",$des,$ff,$bin,$time,$conn,"Unita'","immobiliari","FTTH","0000FFFF"); break;
  case "4011": show4("areeaai","areeaai",$des,$ff,$bin,$time,$conn,"Aree","produttive","connesse","0000FFFF"); break;
  case "4012": show5("aziendeaai","aziendeaai",$des,$ff,$bin,$time,$conn,"Aziende","connesse","0000FFFF"); break;
  case "4013": show4("pal","pal",$des,$ff,$bin,$time,$conn,"Sedi","connesse","Rete Lepida","4B0080FF"); break;
  case "4014": show4("man","man",$des,$ff,$bin,$time,$conn,"Sedi","MAN","Rete Lepida","4B0080FF"); break;
  case "4015": show5("sensori","sensori",$des,$ff,$bin,$time,$conn,"Totale","sensori","FF00FFFF"); break;

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
  $tt=(int)((time()-25200)/86400);
  $query=oci_parse($conn,"select $par from $table where istat='$istat' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=$row[0];
  oci_free_statement($query);
  return $aux;
}

function show4($table,$par,$des,$ff,$bin,$time,$conn,$row1,$row2,$row3,$col1){
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 5 FFFFFFFF 00000000 1 $row1\n");
  fprintf($fp,"1 -2 15 FFFFFFFF 00000000 1 $row2\n");
  fprintf($fp,"1 -2 25 $col1 00000000 1 $row3\n");
  $aux=show3($table,$par,"00008",$conn);
  fprintf($fp,"1 -2 46 $col1 00000000 2 %s\n",($aux<3)?"*":number_format($aux,0,",","."));
  fprintf($fp,"2 0 61 63 63 $col1\n");
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
}

function show5($table,$par,$des,$ff,$bin,$time,$conn,$row1,$row2,$col1){
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 5 FFFFFFFF 00000000 1 $row1\n");
  fprintf($fp,"1 -2 15 $col1 00000000 1 $row2\n");
  $aux=show3($table,$par,"00008",$conn);
  fprintf($fp,"1 -2 46 $col1 00000000 2 %s\n",($aux<3)?"*":number_format($aux,0,",","."));
  fprintf($fp,"2 0 61 63 63 $col1\n");
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
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

function show10($table,$par,$title,$istat,$sovra,$des,$ff,$bin,$time,$conn,$privacy){
  $tt=(int)((time()-25200)/86400);
  $query=oci_parse($conn,"select $par from $table where istat='$istat' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=$row[0];
  oci_free_statement($query);
  $query=oci_parse($conn,"select $par from $table where istat='00008' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$reraux=$row[0];
  oci_free_statement($query);
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 0 FFFFFFFF 00000000 2 %s\n",$title);
  if($sovra<>""){
    $query=oci_parse($conn,"select $par from $table where istat='$sovra' and tt=$tt");
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

function show11($table,$par,$title,$istat,$sovra,$des,$ff,$bin,$time,$conn,$privacy,$bar){
  $tt=(int)((time()-25200)/86400);
  $query=oci_parse($conn,"select $par from $table where istat='$istat' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$aux=$row[0];
  oci_free_statement($query);
  $query=oci_parse($conn,"select $par from $table where istat='00008' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$reraux=$row[0];
  oci_free_statement($query);
  $fp=fopen($des,"w");
  fprintf($fp,"1 -2 0 FFFFFFFF 00000000 2 %s\n",$title);
  if($sovra<>""){
    $query=oci_parse($conn,"select $par from $table where istat='$sovra' and tt=$tt");
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
  fprintf($fp,"2 63 00 63 %02d 999999FF\n",$bar*4+3);
  fclose($fp);
  shell_exec("tmp/write2 $des $ff; tmp/convert3 $ff $time $bin");
  return;
}

?>
