FROM node:22-alpine AS build
WORKDIR /app
COPY package.json package-lock.json ./
RUN npm ci
COPY angular.json tsconfig*.json ./
COPY src ./src
COPY tests ./tests
RUN npm test && npm run build

FROM nginx:stable-alpine
ARG REVISION=unknown
LABEL org.opencontainers.image.source="https://github.com/sabrinahelfenberger-Wusel/PinkRobin" \
      org.opencontainers.image.revision="$REVISION"
COPY nginx.conf /etc/nginx/conf.d/default.conf
COPY 40-access-log-directory.sh /docker-entrypoint.d/40-access-log-directory.sh
RUN chmod 755 /docker-entrypoint.d/40-access-log-directory.sh
COPY --from=build /app/dist/pinkrobin-web/browser/ /usr/share/nginx/html/
RUN printf '%s\n' "$REVISION" > /usr/share/nginx/html/version.txt
HEALTHCHECK --interval=10s --timeout=3s --start-period=5s --retries=3 \
  CMD wget -q -O /dev/null http://127.0.0.1/healthz || exit 1
EXPOSE 80
