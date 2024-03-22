
#ifndef __CLUSTER_MACRO_H__
#define __CLUSTER_MACRO_H__
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/support/ZclString.h>

#define ZAP_GET_CLUSTER_VERSION(cluster) ZCL_CLUSTER_#cluster
#define ZAP_DEVICE_TYPE(type) {type, DEVICE_VERSION_DEFAULT}
#define ZAP_MASK 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)
#define ZAP_EMPTY ZAP_EMPTY_DEFAULT()

#define ZAP_CHAR_STRING ZAP_TYPE(CHAR_STRING)
#define ZAP_ARRAY ZAP_TYPE(ARRAY)
#define ZAP_BOOL  ZAP_TYPE(BOOLEAN)
#define ZAP_INT8  ZAP_TYPE(INT8S)
#define ZAP_INT16 ZAP_TYPE(INT16S)
#define ZAP_INT24 ZAP_TYPE(INT24S)
#define ZAP_INT32 ZAP_TYPE(INT32S)
#define ZAP_BITMAP8  ZAP_TYPE(BITMAP8)
#define ZAP_BITMAP16 ZAP_TYPE(BITMAP16)
#define ZAP_BITMAP24 ZAP_TYPE(BITMAP24)
#define ZAP_BITMAP32 ZAP_TYPE(BITMAP32)
#define ZAP_ENUM8 ZAP_TYPE(ENUM8)
#define ZAP_ENUM16 ZAP_TYPE(ENUM16)
#define ZAP_ENUM24 ZAP_TYPE(ENUM24)
#define ZAP_ENUM32 ZAP_TYPE(ENUM32)

#define ZAP_END_ELEMENT { ZAP_EMPTY, 0xFFFD, 2, ZAP_INT16, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) }
#define ZAP_RECORD(id, val, type) { ZAP_EMPTY, id, val, type, ZAP_MASK }
#define ZAP_BASIC(featuremap, revision) \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::GeneratedCommandList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY), \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::AcceptedCommandList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY), \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::EventList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY), \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::AttributeList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY), \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::FeatureMap::Id, featuremap, ZAP_BITMAP32), \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::ClusterRevision::Id, revision, ZAP_INT16)

#endif // __CLUSTER_MACRO_H__
