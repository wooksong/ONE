/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __ONERT_BACKEND_TRAIN_TENSOR_REGISTRY__
#define __ONERT_BACKEND_TRAIN_TENSOR_REGISTRY__

#include <backend/ITensorRegistry.h>

#include "Tensor.h"

namespace onert
{
namespace backend
{
namespace train
{

class TensorRegistry : public PortableTensorRegistryTemplate<Tensor>
{
public:
  ITensor *getITensor(const ir::OperandIndex &ind) override
  {
    auto tensor = _trainable.find(ind);
    if (tensor != _trainable.end())
      return tensor->second.get();

    return PortableTensorRegistryTemplate<Tensor>::getITensor(ind);
  }

  ITensor *getNativeITensor(const ir::OperandIndex &ind) override
  {
    ITensor *tensor = getTrainableTensor(ind);
    if (tensor == nullptr)
      tensor = getNonConstTensor(ind);
    return tensor;
  }

  IPortableTensor *getPortableTensor(const ir::OperandIndex &ind)
  {
    auto tensor = _trainable.find(ind);
    if (tensor != _trainable.end())
    {
      if (tensor->second)
        return tensor->second.get();
    }
    return PortableTensorRegistryTemplate<Tensor>::getPortableTensor(ind);
  }

  Tensor *getNonConstTensor(const ir::OperandIndex &ind)
  {
    return PortableTensorRegistryTemplate<Tensor>::getNativeTensor(ind);
  }

  TrainableTensor *getTrainableTensor(const ir::OperandIndex &ind)
  {
    auto tensor = _trainable.find(ind);
    if (tensor != _trainable.end())
      return tensor->second.get();

    return nullptr;
  }

  bool setMigrantTensor(const ir::OperandIndex &ind, IPortableTensor *tensor) override
  {
    assert(tensor != nullptr);
    auto itr = _trainable.find(ind);
    if (itr != _trainable.end())
      throw std::runtime_error{
        "Tried to set a migrant tensor but a trainable tensor already exists."};

    return PortableTensorRegistryTemplate<Tensor>::setMigrantTensor(ind, tensor);
  }

  void setNonConstTensor(const ir::OperandIndex &ind, std::unique_ptr<Tensor> tensor)
  {
    assert(tensor != nullptr);
    auto itr = _trainable.find(ind);
    if (itr != _trainable.end())
      throw std::runtime_error{
        "Tried to set a non const tensor but a trainable tensor already exists."};

    PortableTensorRegistryTemplate<Tensor>::setNativeTensor(ind, std::move(tensor));
  }

  void setTrainableTensor(const ir::OperandIndex &ind, std::unique_ptr<TrainableTensor> tensor)
  {
    assert(tensor != nullptr);
    auto itr = _trainable.find(ind);
    if (itr != _trainable.end())
      throw std::runtime_error{
        "Tried to set a trainable tensor but a trainable tensor already exists."};

    if (PortableTensorRegistryTemplate<Tensor>::getITensor(ind) != nullptr)
      throw std::runtime_error{
        "Tried to set a trainable tensor but another tensor already exists."};

    _trainable[ind] = std::move(tensor);
  }

  const ir::OperandIndexMap<std::unique_ptr<TrainableTensor>> &trainable_tensors()
  {
    return _trainable;
  }
  const ir::OperandIndexMap<std::unique_ptr<Tensor>> &nonconst_tensors()
  {
    return PortableTensorRegistryTemplate<Tensor>::native_tensors();
  }

private:
  using PortableTensorRegistryTemplate<Tensor>::native_tensors;
  using PortableTensorRegistryTemplate<Tensor>::getNativeTensor;
  using PortableTensorRegistryTemplate<Tensor>::setNativeTensor;

private:
  ir::OperandIndexMap<std::unique_ptr<TrainableTensor>> _trainable;
};

} // namespace train
} // namespace backend
} // namespace onert

#endif // __ONERT_BACKEND_TRAIN_TENSOR_REGISTRY__
