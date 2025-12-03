/** @type {import('next').NextConfig} */
const nextConfig = {
  reactStrictMode: true,
  // Aumentar limite de tamanho para upload de imagens
  api: {
    bodyParser: {
      sizeLimit: '10mb',
    },
  },
}

module.exports = nextConfig

