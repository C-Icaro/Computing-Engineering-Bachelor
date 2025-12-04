// Armazenamento compartilhado para a última imagem
// Em produção, isso deveria usar um banco de dados ou storage
// NOTA: Em ambiente serverless, o estado pode não ser compartilhado entre instâncias diferentes
let latestImage = null;

export function getLatestImage() {
  console.log('getLatestImage chamado:', {
    hasImage: !!latestImage,
    filename: latestImage?.filename
  });
  return latestImage;
}

export function setLatestImage(imageData) {
  console.log('setLatestImage chamado:', {
    filename: imageData?.filename,
    decision: imageData?.decision,
    timestamp: imageData?.timestamp,
    imageUrlLength: imageData?.imageUrl?.length
  });
  latestImage = imageData;
  console.log('latestImage atualizado com sucesso');
}

