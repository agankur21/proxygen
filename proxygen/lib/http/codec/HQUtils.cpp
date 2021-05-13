/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HQUtils.h>

#include <folly/Overload.h>

namespace proxygen { namespace hq {

const quic::StreamId kSessionStreamId = std::numeric_limits<uint64_t>::max();

// Ingress Sett.hngs Default Values
const uint64_t kDefaultIngressHeaderTableSize = 0;
const uint64_t kDefaultIngressNumPlaceHolders = 0;
const uint64_t kDefaultIngressMaxHeaderListSize = 1u << 17;
const uint64_t kDefaultIngressQpackBlockedStream = 0;

// Egress Settings Default Values
const uint64_t kDefaultEgressHeaderTableSize = 4096;
const uint64_t kDefaultEgressNumPlaceHolders = 16;
const uint64_t kDefaultEgressMaxHeaderListSize = 1u << 17;
const uint64_t kDefaultEgressQpackBlockedStream = 100;

proxygen::ErrorCode hqToHttpErrorCode(HTTP3::ErrorCode err) {
  switch (err) {
    case HTTP3::ErrorCode::HTTP_NO_ERROR:
      return ErrorCode::NO_ERROR;
    case HTTP3::ErrorCode::HTTP_REQUEST_REJECTED:
      return ErrorCode::REFUSED_STREAM;
    case HTTP3::ErrorCode::HTTP_INTERNAL_ERROR:
      return ErrorCode::INTERNAL_ERROR;
    case HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED:
      return ErrorCode::CANCEL;
    case HTTP3::ErrorCode::HTTP_CONNECT_ERROR:
      return ErrorCode::CONNECT_ERROR;
    case HTTP3::ErrorCode::HTTP_EXCESSIVE_LOAD:
      return ErrorCode::ENHANCE_YOUR_CALM;
    case HTTP3::ErrorCode::HTTP_VERSION_FALLBACK:
      return ErrorCode::INTERNAL_ERROR;
    case HTTP3::ErrorCode::HTTP_CLOSED_CRITICAL_STREAM:
    case HTTP3::ErrorCode::HTTP_MISSING_SETTINGS:
    case HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED:
    case HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR:
    case HTTP3::ErrorCode::HTTP_FRAME_ERROR:
    case HTTP3::ErrorCode::HTTP_ID_ERROR:
    case HTTP3::ErrorCode::HTTP_SETTINGS_ERROR:
    case HTTP3::ErrorCode::HTTP_INCOMPLETE_REQUEST:
    case HTTP3::ErrorCode::HTTP_MESSAGE_ERROR:
      return ErrorCode::PROTOCOL_ERROR;
    default:
      return ErrorCode::INTERNAL_ERROR;
  }
}

ProxygenError toProxygenError(quic::QuicErrorCode error, bool fromPeer) {
  switch (error.type()) {
    case quic::QuicErrorCode::Type::ApplicationErrorCode:
      if (*error.asApplicationErrorCode() ==
          HTTP3::ErrorCode::GIVEUP_ZERO_RTT) {
        return kErrorEarlyDataFailed;
      }
      return fromPeer ? kErrorConnectionReset : kErrorConnection;
    case quic::QuicErrorCode::Type::LocalErrorCode:
      return kErrorShutdown;
    case quic::QuicErrorCode::Type::TransportErrorCode:
      return kErrorConnectionReset;
  }
  folly::assume_unreachable();
}

folly::Optional<hq::SettingId> httpToHqSettingsId(proxygen::SettingsId id) {
  switch (id) {
    case proxygen::SettingsId::HEADER_TABLE_SIZE:
      return hq::SettingId::HEADER_TABLE_SIZE;
    case proxygen::SettingsId::MAX_HEADER_LIST_SIZE:
      return hq::SettingId::MAX_HEADER_LIST_SIZE;
    case proxygen::SettingsId::_HQ_QPACK_BLOCKED_STREAMS:
      return hq::SettingId::QPACK_BLOCKED_STREAMS;
    default:
      return folly::none; // this setting has no meaning in HQ
  }
  return folly::none;
}

folly::Optional<proxygen::SettingsId> hqToHttpSettingsId(hq::SettingId id) {
  switch (id) {
    case hq::SettingId::HEADER_TABLE_SIZE:
      return proxygen::SettingsId::HEADER_TABLE_SIZE;
    case hq::SettingId::MAX_HEADER_LIST_SIZE:
      return proxygen::SettingsId::MAX_HEADER_LIST_SIZE;
    case hq::SettingId::QPACK_BLOCKED_STREAMS:
      return proxygen::SettingsId::_HQ_QPACK_BLOCKED_STREAMS;
  }
  return folly::none;
}

}} // namespace proxygen::hq
