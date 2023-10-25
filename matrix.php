<?php

include "data.php";

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

function show1($table,$par,$title,$istat,$sovra,$des,$ff,$bin,$time,$conn){
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
  fprintf($fp,"000000\n");
  fprintf($fp,"-2 00 FFFFFF 02 %s\n",$title);
  if($sovra<>""){
    $query=oci_parse($conn,"select $par from $table where istat='$sovra'");
    oci_execute($query);
    $row=oci_fetch_row($query);
    @$sovraaux=$row[0];
    oci_free_statement($query);
    fprintf($fp,"-2 29 FF0000 01 Unione\n");
    fprintf($fp,"-2 38 FF00FF 02 %s\n",number_format($sovraaux,0,",","."));
    $delta=0;
  }
  else $delta=7;
  fprintf($fp,"-2 %02d FF0000 01 Comune\n",10+$delta);
  fprintf($fp,"-2 %02d FF00FF 02 %s\n",19+$delta,number_format($aux,0,",","."));
  fprintf($fp,"-2 %02d FF0000 01 Regione\n",48-$delta);
  fprintf($fp,"-2 %02d FF00ff 02 %s\n",57-$delta,number_format($reraux,0,",","."));
  fclose($fp);
  shell_exec("tmp/write $des 4 $ff; tmp/convert3 $ff $time $bin");
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

function show3($base,$tot,$ip,$bin,$time,$conn){
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
  $vf=$base + (($next<$tot) ? $next : rand(0,$tot-1));  
  $name=sprintf("tmp/image/%04d.ff",$vf);
  shell_exec("tmp/convert3 $name $time $bin");
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
  case "0001": show1("uiftth","uiftth","FTTH bianche",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0002": show1("areeaai","areeaai","AAI Aree",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0003": show1("aziendeaai","aziendeaai","AAI Aziende",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0004": show1("scuole","scuole","Scuole 1G",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0005": show1("pal","pal","PAL rete",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0006": show1("man","man","MAN rete",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0007": show1("apwifi","apwifi","Punti WiFi",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0008": show1("userwifi","userwifi","Utenti WiFi",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0009": show1("attivifse","attivi","Attivi FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0010": show1("accessifse","accessi","Accessi FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0011": show1("scaricatifse","scaricati","Scaricati FSE",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0012": show1("attivazionilepidaid","attivazioni","Attivazioni ID",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0013": show1("accessilepidaid","accessi","Accessi ID",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;
  case "0014": show1("sportellilepidaid","sportelli","Sportelli ID",$istat,$sovra,$des,$ff,$bin,$time,$conn); break;

  case "8001": shell_exec("tmp/convert3 tmp/image/L001.ff $time $bin"); break;
  case "8002": shell_exec("tmp/convert3 tmp/image/L002.ff $time $bin"); break;
  case "8003": shell_exec("tmp/convert3 tmp/image/L003.ff $time $bin"); break;
  case "8004": shell_exec("tmp/convert3 tmp/image/L004.ff $time $bin"); break;
  case "8005": shell_exec("tmp/convert3 tmp/image/L005.ff $time $bin"); break;
  case "8006": shell_exec("tmp/convert3 tmp/image/L006.ff $time $bin"); break;

  case "9001": show2(1,76,$ip,$bin,$time,$conn); break;
  case "9002": show2(1001,28,$ip,$bin,$time,$conn); break;
  case "9003": show2(2001,258,$ip,$bin,$time,$conn); break;
  case "9103": show3(2001,258,$ip,$bin,$time,$conn); break;

  case "7001":
  $aux=mysplit($ente,12);
  $fp=fopen($des,"w");
  fprintf($fp,"000000\n");
  fprintf($fp,"-2 00 FF0000 03 %sZ\n",date("d.m.y H:i"));
  fprintf($fp,"-2 06 FFFFFF 03 %s %s\n",$ip,$ser);
  fprintf($fp,"00 12 FFFF00 03 Istat\n");
  fprintf($fp,"00 18 0000FF 03 %s\n",$istat);
  if($sovra<>""){
    fprintf($fp,"-1 12 00FFFF 03 Unione\n");
    fprintf($fp,"-1 18 FF0000 03 %s\n",$sovra);
  }
  fprintf($fp,"-2 32 FFFFFF 02 %s\n",$aux[0]);
  fprintf($fp,"-2 40 FFFFFF 02 %s\n",@$aux[1]);
  fprintf($fp,"-2 48 FFFFFF 02 %s\n",@$aux[2]);
  fprintf($fp,"-2 56 FFFFFF 02 %s\n",@$aux[3]);
  fclose($fp);
  shell_exec("tmp/write $des 4 $ff; tmp/convert3 $ff $time $bin");
  break;
    
  default: 
  $fp=fopen($des,"w");
  fprintf($fp,"000000\n");
  fprintf($fp,"-2 00 FF0000 02 ser %s\n",$ser);
  fprintf($fp,"-2 10 00FF00 02 seq %s\n",$seq);
  fprintf($fp,"-2 20 0000FF 02 nseq %d\n",$nseq);
  fprintf($fp,"-2 30 FFFF00 02 mid %d\n",$mid);
  fprintf($fp,"-2 40 FF00FF 02 screen %s\n",$screen);
  fprintf($fp,"-2 50 00FFFF 02 time %d\n",$time);
  fclose($fp);
  shell_exec("tmp/write $des 4 $ff; tmp/convert3 $ff 10 $bin");
  break;
}

$len=filesize($bin);
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile($bin);

oci_close($conn);

?>
