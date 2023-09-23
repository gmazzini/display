<?php

$conn=mysqli_connect("127.0.0.1","matrix","matrix123","matrix");

$res=mysqli_query($conn,"select distinct(sovra) from idistat where sovra<>''");
for($ss=0;;$ss++){
  $row=mysqli_fetch_array($res,MYSQLI_NUM);
  if($row==null)break;
  $sovra[$ss]=$row[0];
}
mysqli_free_result($res);

function fai1($conn,$table,$field,$url,$sovra,$ss){
  echo "$table\n";
  mysqli_query($conn,"delete from $table");
  $aux=json_decode(file_get_contents("$url"),true);
  foreach($aux["dati"] as $k => $v){
    $kk=substr($k,1,5);
    $vv=$v["$field"];
    mysqli_query($conn,"insert into $table values ('$kk','$vv')");
  }
  mysqli_query($conn,"insert into $table select '00008',sum($field) from $table");
  for($i=0;$i<$ss;$i++)mysqli_query($conn,"insert ignore into $table select idistat.sovra,sum($field) from $table,idistat where $table.istat=idistat.istat and idistat.sovra='$sovra[$i]'");
}

function fai2($conn,$base,$table,$field,$url,$sovra,$ss){
  echo "$table\n";
  mysqli_query($conn,"delete from $table");
  $aux=explode("\n",file_get_contents("$url"));
  for($i=$base;;$i++){
    if(!isset($aux[$i]))break;
    $aa=explode(",",$aux[$i]);
    if(strlen($aa[0])<5)continue;
    $kk=substr($aa[0],1,5);
    if(!is_numeric($kk))continue;
    $vv=substr($aa[1],1,strlen($aa[1])-2);
    mysqli_query($conn,"insert into $table values ('$kk',0)");
    mysqli_query($conn,"update $table set $field=$field+$vv where istat='$kk'");
  }
  mysqli_query($conn,"insert into $table select '00008',sum($field) from $table");
  for($i=0;$i<$ss;$i++)mysqli_query($conn,"insert ignore into $table select idistat.sovra,sum($field) from $table,idistat where $table.istat=idistat.istat and idistat.sovra='$sovra[$i]'");
}

function fai3($conn,$base,$table,$field,$url,$sovra,$ss){
  echo "$table\n";
  mysqli_query($conn,"delete from $table");
  $aux=explode("\n",file_get_contents("$url"));
  for($i=$base;;$i++){
    if(!isset($aux[$i]))break;
    if(strlen($aux[$i])<5)continue;
    $kk=substr($aux[$i],1,5);
    if(!is_numeric($kk))continue;
    mysqli_query($conn,"insert ignore into $table values ('$kk',0)");
    mysqli_query($conn,"update $table set $field=$field+1 where istat='$kk'");
  }
  mysqli_query($conn,"insert into $table select '00008',sum($field) from $table");
  for($i=0;$i<$ss;$i++)mysqli_query($conn,"insert ignore into $table select idistat.sovra,sum($field) from $table,idistat where $table.istat=idistat.istat and idistat.sovra='$sovra[$i]'");
}

echo "istatente\n";
mysqli_query($conn,"delete from istatente");
$aux=explode("\n",file_get_contents("https://docs.google.com/spreadsheets/d/1DTngQUDqQgcYhA4S1iOW3jGuj-nOO-98opbXeUC-ffA/gviz/tq?tq=select%20A%2CB%2CD&tqx=out:csv&gid=0"));
for($i=1;;$i++){
  if(!isset($aux[$i]))break;
  $aa=explode(",",$aux[$i]);
  if(strlen($aa[0])<5)continue;
  $kk=substr($aa[0],1,5);
  if(!is_numeric($kk))continue;
  $vv=substr($aa[1],1,strlen($aa[1])-2);
  $qq=substr($aa[2],1,strlen($aa[2])-2);
  mysqli_query($conn,"insert into istatente values ('$kk','$vv')");
  mysqli_query($conn,"update idistat set sovra='$qq' where istat='$kk'");
}

fai1($conn,"attivifse","attivi","https://dati.fascicolo-sanitario.it/rest/attivi/comune",$sovra,$ss);
fai1($conn,"accessifse","accessi","https://dati.fascicolo-sanitario.it/rest/accessi/comune",$sovra,$ss);
fai1($conn,"scaricatifse","scaricati","https://dati.fascicolo-sanitario.it/rest/documenti/comune",$sovra,$ss);
fai1($conn,"attivazionilepidaid","attivazioni","https://dati.fascicolo-sanitario.it/rest/lepidaid/attivazioni/comune",$sovra,$ss);
fai1($conn,"accessilepidaid","accessi","https://dati.fascicolo-sanitario.it/rest/lepidaid/accessi/comune",$sovra,$ss);
fai1($conn,"sportellilepidaid","sportelli","https://dati.fascicolo-sanitario.it/rest/lepidaid/sportelli/comune",$sovra,$ss);
fai2($conn,3,"uiftth","uiftth","https://docs.google.com/spreadsheets/d/1Nk39CPjf9Lu_UQ_zUnY97cqYZ5Vh7K00owrw-XeSgHM/gviz/tq?tq=select%20B%2CD&tqx=out:csv",$sovra,$ss);
fai3($conn,1,"man","man","https://docs.google.com/spreadsheets/d/1DEs7yoAfJ6wK9L-V5kYoArEeP-g110NgPMU0DDFv9EE/gviz/tq?tq=select%20V%20where%20K%3D%27MAN%27&tqx=out:csv&gid=902689105",$sovra,$ss);
fai3($conn,1,"pal","pal","https://docs.google.com/spreadsheets/d/1DEs7yoAfJ6wK9L-V5kYoArEeP-g110NgPMU0DDFv9EE/gviz/tq?tq=select%20V&tqx=out:csv&gid=1797209276",$sovra,$ss);
fai3($conn,1,"scuole","scuole","https://docs.google.com/spreadsheets/d/10xN81W5Dd8LRjVOm_FJ0ubzgJ1EWJ4Vfi8P3iVZdBho/gviz/tq?tq=select%20A%20where%20U%3D%27BULBUL%27&tqx=out:csv&gid=566741345",$sovra,$ss);
fai3($conn,1,"areeaai","areeaai","https://docs.google.com/spreadsheets/d/1cgCtacbWsm7wybTp8cA7wWBFo9bZOSc7JAnlV99K-O0/gviz/tq?tq=select%20G&tqx=out:csv&gid=566741345",$sovra,$ss);
fai2($conn,1,"aziendeaai","aziendeaai","https://docs.google.com/spreadsheets/d/1cgCtacbWsm7wybTp8cA7wWBFo9bZOSc7JAnlV99K-O0/gviz/tq?tq=select%20G%2CF&tqx=out:csv&gid=566741345",$sovra,$ss);
fai2($conn,1,"apwifi","apwifi","https://docs.google.com/spreadsheets/d/1cgCtacbWsm7wybTp8cA7wWBFo9bZOSc7JAnlV99K-O0/gviz/tq?tq=select%20H%2CI&tqx=out:csv&gid=1373772362",$sovra,$ss);

mysqli_close($conn);

?>
