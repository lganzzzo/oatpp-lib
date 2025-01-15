<div align="center">
  <img alt="Oat++ Logo" src="https://raw.githubusercontent.com/lganzzzo/oatpp-website-res/master/logo_x400.png" width="200px"/>
</div>
<p>&nbsp;</p>

<p align="center">
  <a href="https://dev.azure.com/lganzzzo/lganzzzo/_build?definitionId=1">
    <img src="https://dev.azure.com/lganzzzo/lganzzzo/_apis/build/status/oatpp.oatpp" alt="oatpp build status" />
  </a>
  <a href="https://gitter.im/oatpp-framework/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge">
    <img src="https://badges.gitter.im/oatpp-framework/Lobby.svg" alt="Join the chat at https://gitter.im/oatpp-framework/Lobby" />
  </a>
</p>

# Oat++ 

Oat++ is a modern Web Framework for C++. 
It's fully loaded and contains all the necessary components for effective production-level development. 
It's also light and has a small memory footprint.

## News  

- ⚠️ Attention! Oat++ main repo is bumping its version to 1.4.0. While 1.4.0 is **IN DEVELOPMENT** use `1.3.0-latest` tag.
- Follow the [changelog](https://github.com/oatpp/oatpp/blob/master/changelog/1.4.0.md) for news and features in version `1.4.0`.
- Consider supporting Oat++ via the [GitHub sponsors](https://github.com/sponsors/oatpp) page.

### New ORM integrations:

- [oatpp-mysql](https://github.com/Atarasin/oatpp-mysql) by [@Atarasin](https://github.com/Atarasin)
- [oatpp-mariadb](https://github.com/lilltiger/oatpp-mariadb) by [@lilltiger](https://github.com/lilltiger)

## Start

- [Get Started](https://oatpp.io/docs/start/)
- [Build For Unix/Linux](https://oatpp.io/docs/installation/unix-linux/)
- [Build For Windows](https://oatpp.io/docs/installation/windows/)
- [Examples](#examples)

## Support the Project
Maintaining and developing this project requires significant time and resources. If you enjoy using it and want to support its continued development, please consider supporting us through GitHub Sponsors.

**[Become a Sponsor on GitHub](https://github.com/sponsors/oatpp)** 

Every contribution, big or small, helps us keep improving and maintaining the project.
Thank you for your support!

## About

- [Website](https://oatpp.io/)
- [Supported Platforms](https://oatpp.io/supported-platforms/)
- Latest Benchmarks: [5 Million WebSockets](https://oatpp.io/benchmark/websocket/5-million/)
- [Contributing to Oat++](CONTRIBUTING.md)




## Quick Overview

**Shortcuts:**

- [Oat++ High Level Overview](https://oatpp.io/docs/start/high-level-overview/) - Get a quick overview of Oat++ features.
- [Example Project](https://github.com/oatpp/example-crud) - A complete example of a "CRUD" service (UserService) built with Oat++. REST + Swagger-UI + SQLite.

### Build Powerful API And Document It With Swagger-UI

See [ApiController](https://oatpp.io/docs/components/api-controller/) for more details.

```cpp
ENDPOINT_INFO(getUserById) {
  info->summary = "Get one User by userId";

  info->addResponse<Object<UserDto>>(Status::CODE_200, "application/json");
  info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");
  info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");

  info->pathParams["userId"].description = "User Identifier";
}
ENDPOINT("GET", "users/{userId}", getUserById,
         PATH(Int32, userId))
{
  return createDtoResponse(Status::CODE_200, m_userService.getUserById(userId));
}
``` 

### Access Databases And Keep Your Data Consistent

See [Oat++ ORM](https://oatpp.io/docs/components/orm/) for more details.

```cpp
QUERY(createUser,
      "INSERT INTO users (username, email, role) VALUES (:username, :email, :role);",
      PARAM(oatpp::String, username), 
      PARAM(oatpp::String, email), 
      PARAM(oatpp::Enum<UserRoles>::AsString, role))
```

## Join Our Community

- [Gitter](https://gitter.im/oatpp-framework/Lobby) - Talk to Oat++ developers and to other Oat++ users.
- [Twitter](https://twitter.com/oatpp_io) - Follow Oat++ on Twitter.
- [Reddit](https://www.reddit.com/r/oatpp/) - Follow Oat++ subreddit.
- [StackOverflow (new)](https://stackoverflow.com/questions/tagged/oat%2b%2b) - Post a Question.

## Examples

### REST-API

- [REST Service](https://github.com/oatpp/example-crud) - A complete example of a "CRUD" service (UserService) built with Oat++. REST + Swagger-UI + SQLite.
- [REST Client](https://github.com/oatpp/example-api-client) - Example project of how-to use Retrofit-like client wrapper (ApiClient) and how it works.

### WebSocket

- [Can Chat](https://github.com/lganzzzo/canchat) - Feature-complete rooms-based chat for tens of thousands of users. Client plus Server.
- [WebSocket](https://github.com/oatpp/example-websocket) - Collection of oatpp WebSocket examples.
- [YUV Websocket Stream](https://github.com/oatpp/example-yuv-websocket-stream) - Example project how-to create a YUV image stream from a V4L device (i.E. Webcam) using websockets.

### Databases

- [SQLite](https://github.com/oatpp/example-crud) - A complete example of a "CRUD" service. REST + Swagger-UI + SQLite.
- [PostgreSQL](https://github.com/oatpp/example-postgresql) - Example of a production-grade entity service storing information in PostgreSQL. With Swagger-UI and configuration profiles.
- [MongoDB](https://github.com/oatpp/example-mongodb) - Example project how to work with MongoDB using **oatpp-mongo** mondule. The project is a web-service with basic CRUD and Swagger-UI.
 
### IoT

- [Example-IoT-Hue](https://github.com/oatpp/example-iot-hue-ssdp) - Example project how-to create a Philips Hue compatible REST-API that is discovered and controllable by Hue compatible Smart-Home devices like Amazon Alexa or Google Echo.

### Streaming

- [HTTP Live Streaming Server](https://github.com/oatpp/example-hls-media-stream) - Example project on how to build an HLS-streaming server using Oat++ asynchronous API.
- [YUV Websocket Stream](https://github.com/oatpp/example-yuv-websocket-stream) - Example project how-to create a YUV image stream from a V4L device (i.E. Webcam) using websockets.

### TLS

- [TLS With Libressl](https://github.com/oatpp/example-libressl) - Example project how-to setup secure connection and serve via HTTPS.

### Microservices

- [Consul Integration](https://github.com/oatpp/example-consul) - Example project on how to use [oatpp::consul::Client](https://oatpp.io/api/latest/oatpp-consul/rest/Client/). Consul integration.
- [Microservices](https://github.com/oatpp/example-microservices) - Example project on how to build microservices with Oat++, 
and example on how to consolidate those microservices using [monolithization](https://oatpp.io/docs/monolithization/) technique.

### Asynchronous API

- [Async Service](https://github.com/oatpp/example-async-api) - Example project on how to use asynchronous API to handle a large number of simultaneous connections.

## Frequently Asked Questions

### Q: "Oat++" name?

- "Oat" is something light, organic, and green. It can be easily cooked and consumed with no effort. 
- "++" gives a hint that it is "something" for C++.

### Q: What is the main area of Oat++ application?

Oat++ is used for many different purposes, from building REST APIs that run on embedded devices to 
building microservices and highly-loaded cloud applications.

But the majority of use cases appears to be in **IoT** and **Robotics**.

### Q: How portable is Oat++?

Theoretically, Oat++ can be **easily** ported everywhere where you have **threads** and **network stack**.
With an additional comparably small effort, it can be ported almost everywhere depending on how
much you strip it and what would be the final binary size.

See [supported platforms](https://oatpp.io/supported-platforms/) for additional info.

### Q: What is the size of a minimal Oat++ application?

About **1Mb**, depending on C/C++ std-lib and oatpp version.

### Q: Which Oat++ API to choose, Simple or Async?

Always choose **Simple API** wherever possible. Simple API is more developed and makes the code cleaner.

Async API is designed for small, specific tasks that run at high concurrency levels ex.:
- Serving file downloads to a large number of concurrent users (1K users and more).
- Streaming to a large number of clients (1K or more).
- Websocket Chat servers.

For all other purposes use simple API.
