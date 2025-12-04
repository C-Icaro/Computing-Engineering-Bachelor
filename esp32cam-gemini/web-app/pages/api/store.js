// Armazenamento compartilhado para a última imagem
// Em produção, isso deveria usar um banco de dados ou storage
let latestImage = null;

export function getLatestImage() {
  return latestImage;
}

export function setLatestImage(imageData) {
  latestImage = imageData;
}

