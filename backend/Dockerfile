FROM mcr.microsoft.com/dotnet/sdk:10.0-alpine AS build
RUN apk add --no-cache cmake build-base
WORKDIR /src/backend
COPY backend/global.json ./
COPY backend/NativeCore.targets ./
COPY backend/src/PinkRobin.Api/PinkRobin.Api.csproj src/PinkRobin.Api/
RUN dotnet restore src/PinkRobin.Api/PinkRobin.Api.csproj
COPY backend/src/ src/
COPY core/ /src/core/
RUN dotnet publish src/PinkRobin.Api/PinkRobin.Api.csproj -c Release --no-restore -o /app/publish

FROM mcr.microsoft.com/dotnet/aspnet:10.0-alpine
RUN apk add --no-cache libstdc++
WORKDIR /app
ARG REVISION=unknown
LABEL org.opencontainers.image.source="https://github.com/sabrinahelfenberger-Wusel/PinkRobin" \
      org.opencontainers.image.revision="$REVISION"
COPY --from=build /app/publish/ ./
ENV ASPNETCORE_HTTP_PORTS=8080
USER $APP_UID
EXPOSE 8080
HEALTHCHECK --interval=10s --timeout=3s --start-period=10s --retries=3 \
  CMD wget -q -O /dev/null http://127.0.0.1:8080/health/live || exit 1
ENTRYPOINT ["dotnet", "PinkRobin.Api.dll"]
