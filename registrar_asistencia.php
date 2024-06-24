<?php
if (isset($_GET['mac'])) {
  $mac = $_GET['mac'];
  $servername = "localhost";
  $username = "root";
  $password = "";
  $dbname = "conexionprueba";

  $conn = new mysqli($servername, $username, $password, $dbname);

  if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
  }

  // Verificar si la MAC existe en la tabla alumno
  $sql = "SELECT * FROM alumno WHERE mac_address='$mac'";
  $result = $conn->query($sql);

  if ($result->num_rows > 0) {
    // Registrar la asistencia en la tabla asistencia
    $sql = "INSERT INTO asistencia (mac_address, hora) VALUES ('$mac', NOW())";
    if ($conn->query($sql) === TRUE) {
      echo "Asistencia registrada con éxito";
    } else {
      echo "Error: " . $sql . "<br>" . $conn->error;
    }
  } else {
    echo "MAC no registrada en la tabla de alumnos";
  }

  $conn->close();
} else {
  echo "No se recibió la MAC";
}
?>
