<?php
if (isset($_GET['mac'])) {
  $mac = $_GET['mac'];
  $servername = "localhost";
  $username = "root";
  $password = "";
  $dbname = "conexionprueba";

  // Crear conexión
  $conn = new mysqli($servername, $username, $password, $dbname);

  // Verificar conexión
  if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
  }

  // Preparar y vincular
  $stmt = $conn->prepare("SELECT nombre, apellido FROM alumno WHERE mac_address=?");
  $stmt->bind_param("s", $mac);
  $stmt->execute();
  $result = $stmt->get_result();

  if ($result->num_rows > 0) {
    $row = $result->fetch_assoc();
    $nombre = $row['nombre'];
    $apellido = $row['apellido'];

    $sql = $conn->prepare("INSERT INTO asistencia (nombre, apellido, hora) VALUES (?, ?, NOW())");
    $sql->bind_param("ss", $nombre, $apellido);
    if ($sql->execute()) {
      echo "Asistencia registrada con éxito para $nombre $apellido";
    } else {
      echo "Error al registrar asistencia: " . $conn->error;
    }
  } else {
    echo "MAC no registrada en la tabla de alumnos, por ende, alumno registrado en otro salón";
  }

  $stmt->close();
  $conn->close();
} else {
  echo "No se recibió la MAC";
}
?>
