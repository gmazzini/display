<?php

$ip=$_GET["ip"];
$ser=$_GET["ser"];
if($ser=="")$ser="0000";
$des="tmp/files/$ip.des";
$ff="tmp/files/$ip.ff";
$bin="tmp/files/$ip.bin";

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

function showme($table,$par,$title,$istat,$sovra,$des,$ff,$bin,$conn){
  $res=mysqli_query($conn,"select $par from $table where istat='$istat'");
  $row=mysqli_fetch_array($res,MYSQLI_NUM);
  @$aux=$row[0];
  mysqli_free_result($res);
  $res=mysqli_query($conn,"select $par from $table where istat='00008'");
  $row=mysqli_fetch_array($res,MYSQLI_NUM);
  @$reraux=$row[0];
  mysqli_free_result($res);
  $fp=fopen($des,"w");
  fprintf($fp,"000000\n");
  fprintf($fp,"-2 00 400040 02 %s\n",$title);
  if($sovra<>""){
    $res=mysqli_query($conn,"select $par from $table where istat='$sovra'");
    $row=mysqli_fetch_array($res,MYSQLI_NUM);
    @$sovraaux=$row[0];
    mysqli_free_result($res);
    fprintf($fp,"-2 29 00FFFF 01 Unione\n");
    fprintf($fp,"-2 38 0000FF 02 %s\n",number_format($sovraaux,0,",","."));
    $delta=0;
  }
  else $delta=7;
  fprintf($fp,"-2 %02d FFFF00 01 Comune\n",10+$delta);
  fprintf($fp,"-2 %02d 00FF00 02 %s\n",19+$delta,number_format($aux,0,",","."));
  fprintf($fp,"-2 %02d FF00FF 01 Regione\n",48-$delta);
  fprintf($fp,"-2 %02d FF0000 02 %s\n",57-$delta,number_format($reraux,0,",","."));
  fclose($fp);
  shell_exec("tmp/write $des 4 $ff; tmp/convert3 $ff 10 $bin");
  return;
}

$conn=mysqli_connect("127.0.0.1","matrix","matrix123","matrix");

$istat=0;
$aux=explode(".",$ip);
if($aux[0]=="10"){
  $aux=explode(".",$ip);
  $id=$aux[1]*256+$aux[2];
  $res=mysqli_query($conn,"select istat,sovra from idistat where '$id'>=idstart and '$id'<=idend");
  $row=mysqli_fetch_array($res,MYSQLI_NUM);
  $istat=$row[0];
  $sovra=$row[1];
  mysqli_free_result($res);
  $res=mysqli_query($conn,"select ente from istatente where istat='$istat'");
  $row=mysqli_fetch_array($res,MYSQLI_NUM);
  $ente=$row[0];
  mysqli_free_result($res);
}

mysqli_query($conn,"insert ignore into session values ('$ip','$ser',0,0,0)");
mysqli_query($conn,"update session set iter=iter+1 where id='$ip'");
mysqli_query($conn,"update session set ser='$ser' where id='$ip'");
$res=mysqli_query($conn,"select iter from session where id='$ip'");
$row=mysqli_fetch_array($res,MYSQLI_NUM);
$sel=$row[0]%17;
mysqli_free_result($res);

switch($sel){

case 0:
  $aux=mysplit($ente,12);
  $fp=fopen($des,"w");
  fprintf($fp,"000000\n");
  fprintf($fp,"-2 00 FF0000 03 %s\n",date("d.m.y H:i"));
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
  shell_exec("tmp/write $des 4 $ff; tmp/convert3 $ff 10 $bin");
  break;

case 1:
  showme("uiftth","uiftth","FTTH bianche",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 2:
  showme("areeaai","areeaai","AAI Aree",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 3:
  showme("aziendeaai","aziendeaai","AAI Aziende",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 4:
  showme("scuole","scuole","Scuole 1G",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 5:
  showme("pal","pal","PAL rete",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 6:
  showme("man","man","MAN rete",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 7:
  showme("apwifi","apwifi","Punti WiFi",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 8:
  showme("userwifi","userwifi","Utenti WiFi",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 9:
  showme("attivifse","attivi","Attivi FSE",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 10:
  showme("accessifse","accessi","Accessi FSE",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 11:
  showme("scaricatifse","scaricati","Scaricati FSE",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 12:
  showme("attivazionilepidaid","attivazioni","Attivazioni ID",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 13:
  showme("accessilepidaid","accessi","Accessi ID",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

case 14:
  showme("sportellilepidaid","sportelli","Sportelli ID",$istat,$sovra,$des,$ff,$bin,$conn);
  break;

// immagine
case 15:
  mysqli_query($conn,"update session set c1=c1+1 where id='$ip'");
  $res=mysqli_query($conn,"select c1 from session where id='$ip'");
  $row=mysqli_fetch_array($res,MYSQLI_NUM);
  $vf=$row[0]%76;
  mysqli_free_result($res);
  $name=sprintf("tmp/img/%03d.ff",$vf);
  shell_exec("tmp/convert3 $name 7 $bin");
  break;

// Logo
case 16:
  $name="tmp/img/logo.ff";
  shell_exec("tmp/convert3 $name 5 $bin");
  break;
}

$len=filesize($bin);
header("Content-Type: application/octet-stream");
header("Content-Length: $len");
readfile($bin);

mysqli_close($conn);

?>
