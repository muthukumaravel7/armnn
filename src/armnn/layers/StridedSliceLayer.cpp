//
// Copyright © 2017 Arm Ltd. All rights reserved.
// SPDX-License-Identifier: MIT
//
#include "StridedSliceLayer.hpp"

#include "LayerCloneBase.hpp"

#include <backendsCommon/WorkloadData.hpp>
#include <backendsCommon/WorkloadFactory.hpp>

#include <boost/numeric/conversion/cast.hpp>

namespace armnn
{

StridedSliceLayer::StridedSliceLayer(const armnn::StridedSliceDescriptor& param, const char* name)
    : LayerWithParameters(1, 1, LayerType::StridedSlice, param, name)
{
}

std::unique_ptr<IWorkload> StridedSliceLayer::CreateWorkload(const IWorkloadFactory& factory) const
{
    StridedSliceQueueDescriptor descriptor;

    descriptor.m_Parameters.m_Begin  = m_Param.m_Begin;
    descriptor.m_Parameters.m_End    = m_Param.m_End;
    descriptor.m_Parameters.m_Stride = m_Param.m_Stride;

    // Optional parameters
    descriptor.m_Parameters.m_BeginMask      = m_Param.m_BeginMask;
    descriptor.m_Parameters.m_EndMask        = m_Param.m_EndMask;
    descriptor.m_Parameters.m_EllipsisMask   = m_Param.m_EllipsisMask;
    descriptor.m_Parameters.m_NewAxisMask    = m_Param.m_NewAxisMask;
    descriptor.m_Parameters.m_ShrinkAxisMask = m_Param.m_ShrinkAxisMask;

    return factory.CreateStridedSlice(descriptor, PrepInfoAndDesc(descriptor));
}

StridedSliceLayer* StridedSliceLayer::Clone(Graph& graph) const
{
    return CloneBase<StridedSliceLayer>(graph, m_Param, GetName());
}

std::vector<TensorShape> StridedSliceLayer::InferOutputShapes(
    const std::vector<TensorShape>& inputShapes) const
{
    ARMNN_ASSERT(inputShapes.size() == 1);

    TensorShape inputShape = inputShapes[0];
    std::vector<unsigned int> outputShape;

    for (unsigned int i = 0; i < inputShape.GetNumDimensions(); i++)
    {
        int stride = m_Param.m_Stride[i];
        int start = m_Param.GetStartForAxis(inputShape, i);
        int stop = m_Param.GetStopForAxis(inputShape, i, start);

        if (m_Param.m_ShrinkAxisMask & (1 << i))
        {
            // If the difference between the start point and the end point of the slice on an axis being shrunk
            // is greater than 1 then throw an error as the output will not be large enough to hold the slice
            if (((m_Param.m_Begin[i] - m_Param.m_End[i]) > 1) || ((m_Param.m_Begin[i] - m_Param.m_End[i]) < -1))
            {
                throw LayerValidationException(
                    "StridedSlice: Attempting to take a larger slice than can fit in inferred output");
            }
            continue;
        }

        int newSize = stride > 0 ? ((stop - start) + stride - 1) / stride :
                                   ((start - stop) - stride - 1) / -stride;

        newSize = std::max(0, newSize);

        outputShape.push_back(boost::numeric_cast<unsigned int>(newSize));
    }

    return std::vector<TensorShape>({
        TensorShape(boost::numeric_cast<unsigned int>(outputShape.size()), &outputShape[0]) });
}

void StridedSliceLayer::ValidateTensorShapesFromInputs()
{
    VerifyLayerConnections(1, CHECK_LOCATION());

    auto inferredShapes = InferOutputShapes({GetInputSlot(0).GetConnection()->GetTensorInfo().GetShape()});

    ARMNN_ASSERT(inferredShapes.size() == 1);

    ConditionalThrowIfNotEqual<LayerValidationException>(
                    "StridedSlice: TensorShape set on OutputSlot[0] does not match the inferred shape.",
                    GetOutputSlot(0).GetTensorInfo().GetShape(),
                    inferredShapes[0]);
}

void StridedSliceLayer::Accept(ILayerVisitor& visitor) const
{
    visitor.VisitStridedSliceLayer(this, GetParameters(), GetName());
}

} // namespace armnn
