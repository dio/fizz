/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/ClientExtensions.h>
#include <fizz/client/PskCache.h>
#include <folly/io/async/test/MockAsyncTransport.h>

namespace fizz {
namespace client {
namespace test {

class MockClientStateMachine : public ClientStateMachine {
 public:
  MOCK_METHOD6(
      _processConnect,
      folly::Optional<Actions>(
          const State&,
          std::shared_ptr<const FizzClientContext> context,
          std::shared_ptr<const CertificateVerifier>,
          folly::Optional<std::string> host,
          folly::Optional<CachedPsk> cachedPsk,
          const std::shared_ptr<ClientExtensions>& extensions));
  Actions processConnect(
      const State& state,
      std::shared_ptr<const FizzClientContext> context,
      std::shared_ptr<const CertificateVerifier> verifier,
      folly::Optional<std::string> host,
      folly::Optional<CachedPsk> cachedPsk,
      const std::shared_ptr<ClientExtensions>& extensions) override {
    return *_processConnect(
        state, context, verifier, host, cachedPsk, extensions);
  }

  MOCK_METHOD2(
      _processSocketData,
      folly::Optional<Actions>(const State&, folly::IOBufQueue&));
  Actions processSocketData(const State& state, folly::IOBufQueue& queue)
      override {
    return *_processSocketData(state, queue);
  }

  MOCK_METHOD2(
      _processAppWrite,
      folly::Optional<Actions>(const State&, AppWrite&));
  Actions processAppWrite(const State& state, AppWrite appWrite) override {
    return *_processAppWrite(state, appWrite);
  }

  MOCK_METHOD2(
      _processEarlyAppWrite,
      folly::Optional<Actions>(const State&, EarlyAppWrite&));
  Actions processEarlyAppWrite(const State& state, EarlyAppWrite appWrite)
      override {
    return *_processEarlyAppWrite(state, appWrite);
  }

  MOCK_METHOD1(_processAppClose, folly::Optional<Actions>(const State&));
  Actions processAppClose(const State& state) override {
    return *_processAppClose(state);
  }
};

template <typename SM>
class MockHandshakeCallbackT : public AsyncFizzClientT<SM>::HandshakeCallback {
 public:
  MOCK_METHOD0(_fizzHandshakeSuccess, void());
  void fizzHandshakeSuccess(AsyncFizzClientT<SM>*) noexcept override {
    _fizzHandshakeSuccess();
  }

  MOCK_METHOD1(_fizzHandshakeError, void(folly::exception_wrapper));
  void fizzHandshakeError(
      AsyncFizzClientT<SM>*,
      folly::exception_wrapper ew) noexcept override {
    _fizzHandshakeError(std::move(ew));
  }
};

using MockHandshakeCallback = MockHandshakeCallbackT<ClientStateMachine>;

class MockAsyncFizzClient : public AsyncFizzClient {
 public:
  MockAsyncFizzClient()
      : AsyncFizzClient(
            folly::AsyncTransportWrapper::UniquePtr(
                new folly::test::MockAsyncTransport()),
            std::make_shared<FizzClientContext>()) {}
  MOCK_METHOD5(
      connect,
      void(
          HandshakeCallback*,
          std::shared_ptr<const CertificateVerifier>,
          folly::Optional<std::string>,
          folly::Optional<std::string>,
          std::chrono::milliseconds));
  MOCK_METHOD0(close, void());
  MOCK_METHOD0(closeWithReset, void());
  MOCK_METHOD0(closeNow, void());
};

class MockPskCache : public PskCache {
 public:
  MOCK_METHOD1(getPsk, folly::Optional<CachedPsk>(const std::string& identity));
  MOCK_METHOD2(putPsk, void(const std::string& identity, CachedPsk));
  MOCK_METHOD1(removePsk, void(const std::string& identity));
};

class MockClientExtensions : public ClientExtensions {
 public:
  MOCK_CONST_METHOD0(getClientHelloExtensions, std::vector<Extension>());
  MOCK_METHOD1(onEncryptedExtensions, void(const std::vector<Extension>&));
};
} // namespace test
} // namespace client
} // namespace fizz
