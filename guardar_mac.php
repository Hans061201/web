<?php
if (isset($_GET['mac'])) {
  $mac = $_GET['mac'];
  $servername = "localhost";
  $username = "root";
  $password = "";
  $dbname = "conexionprueba";
  echo "MAC recibida: " . $mac;
  $conn = new mysqli($servername, $username, $password, $dbname);

  if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
  }

  $sql = "SELECT * FROM alumno WHERE mac_address='$mac'";
  $result = $conn->query($sql);

  if ($result->num_rows == 0) {
    $sql = "INSERT INTO alumno (mac_address) VALUES ('$mac')";
    if ($conn->query($sql) === TRUE) {
      echo "Nuevo registro creado con éxito";
    } else {
      echo "Error: " . $sql . "<br>" . $conn->error;
    }
  } else {
    echo "MAC ya registrada";
  }

  $conn->close();
} else {
  echo "No se recibió la MAC";
}
?>
