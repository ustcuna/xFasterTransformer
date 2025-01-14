// Copyright (c) 2023 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ============================================================================
#pragma once

#include <torch/custom_class.h>
#include <torch/script.h>

#include <string>
#include <vector>

#include "xfastertransformer.h"

struct TorchAutoModel : torch::CustomClassHolder {
public:
    TorchAutoModel(std::string modelPath, std::string dtype) {
        xft::DataType datatype;
        if (dtype == "fp16") {
            datatype = xft::DataType::fp16;
        } else if (dtype == "bf16") {
            datatype = xft::DataType::bf16;
        } else if (dtype == "int8") {
            datatype = xft::DataType::int8;
        } else if (dtype == "int4") {
            datatype = xft::DataType::int4;
        } else if (dtype == "w8a8") {
            datatype = xft::DataType::w8a8;
        } else if (dtype == "nf4") {
            datatype = xft::DataType::nf4;
        } else if (dtype == "bf16_fp16") {
            datatype = xft::DataType::bf16_fp16;
        } else if (dtype == "bf16_int8") {
            datatype = xft::DataType::bf16_int8;
        } else if (dtype == "bf16_w8a8") {
            datatype = xft::DataType::bf16_w8a8;
        } else if (dtype == "bf16_int4") {
            datatype = xft::DataType::bf16_int4;
        } else if (dtype == "bf16_nf4") {
            datatype = xft::DataType::bf16_nf4;
        } else if (dtype == "w8a8_int8") {
            datatype = xft::DataType::w8a8_int8;
        } else if (dtype == "w8a8_int4") {
            datatype = xft::DataType::w8a8_int4;
        } else if (dtype == "w8a8_nf4") {
            datatype = xft::DataType::w8a8_nf4;
        } else {
            throw std::invalid_argument("Invalid DataType");
        }
        model = new xft::AutoModel(modelPath, datatype);
    };

    ~TorchAutoModel() {
        if (model != nullptr) { delete model; }
    }

    int64_t getRank() { return static_cast<int64_t>(model->getRank()); }

    bool isDone() { return model->isDone(); }

    void input(torch::optional<torch::Tensor> inputIds) {
        int batchSize = 0;
        if (model->getRank() == 0) {
            TORCH_CHECK(inputIds.has_value(), "Make sure master's input is not None.")

            batchSize = inputIds.value().size(0);
            int seqLen = inputIds.value().size(1);

            tokenIds.resize(batchSize * seqLen);
            int64_t *p = inputIds.value().data_ptr<int64_t>();
            if (model->getRank() == 0) {
                for (int i = 0; i < batchSize * seqLen; ++i) {
                    tokenIds[i] = static_cast<int>(*p);
                    p += 1;
                }
            }
        }
        model->input(tokenIds, batchSize);
    }

    void config(torch::optional<int64_t> maxLength, torch::optional<int64_t> numBeamsOpt,
            torch::optional<int64_t> numReturnSequencesOpt, torch::optional<double> lenPenaltyOpt,
            torch::optional<bool> earlyStoppingOpt, torch::optional<int64_t> eosTokenIdOpt,
            torch::optional<int64_t> padTokenIdOpt, torch::optional<bool> doSampleOpt,
            torch::optional<double> temperaturOpt, torch::optional<int64_t> topKOpt, torch::optional<double> topPOpt,
            torch::optional<double> repetitionPenaltyOpt,
            torch::optional<std::vector<std::vector<int64_t>>> stopWordsListOpt) {
        TORCH_CHECK(maxLength.has_value(), "Make sure master's maxLen is not None.")
        int maxLen = static_cast<int>(maxLength.value());
        int numBeams = numBeamsOpt.has_value() ? (int)numBeamsOpt.value() : 1;
        int numBeamHypsToKeep = numReturnSequencesOpt.has_value() ? (int)numReturnSequencesOpt.value() : 1;
        float lenPenalty = lenPenaltyOpt.has_value() ? static_cast<float>(lenPenaltyOpt.value()) : 1.0;
        bool doEarlyStopping = earlyStoppingOpt.has_value() ? (bool)earlyStoppingOpt.value() : false;
        int eosTokenId = eosTokenIdOpt.has_value() ? static_cast<int>(eosTokenIdOpt.value()) : -1;
        int padTokenId = padTokenIdOpt.has_value() ? static_cast<int>(padTokenIdOpt.value()) : -1;
        bool doSample = doSampleOpt.has_value() ? (bool)doSampleOpt.value() : false;
        float temperature = temperaturOpt.has_value() ? static_cast<float>(temperaturOpt.value()) : 1.0;
        int topK = topKOpt.has_value() ? (int)topKOpt.value() : 50;
        float topP = topPOpt.has_value() ? static_cast<float>(topPOpt.value()) : 1.0;
        float repetitionPenalty
                = repetitionPenaltyOpt.has_value() ? static_cast<float>(repetitionPenaltyOpt.value()) : 1.0;

        std::vector<std::vector<int>> stopWordsList_int32;
        if (stopWordsListOpt.has_value()) {
            std::vector<std::vector<int64_t>> &stopWordsList = stopWordsListOpt.value();
            stopWordsList_int32.reserve(stopWordsList.size());
            for (const auto &inner_vector : stopWordsList) {
                std::vector<int> converted_vector;
                converted_vector.reserve(inner_vector.size());

                std::transform(inner_vector.begin(), inner_vector.end(), std::back_inserter(converted_vector),
                        [](int64_t value) { return static_cast<int>(value); });

                stopWordsList_int32.emplace_back(converted_vector);
            }
        }

        model->config(maxLen, numBeams, numBeamHypsToKeep, lenPenalty, doEarlyStopping, eosTokenId, padTokenId,
                doSample, temperature, topK, topP, repetitionPenalty, stopWordsList_int32);
    }

    torch::Tensor forward(torch::Tensor &inputIds) {
        int batchSize = inputIds.size(0);
        int seqLen = inputIds.size(1);
        int vocabSize = model->getVocabSize();
        int logitsN = batchSize * seqLen * vocabSize;

        if (model->getRank() == 0) { input(inputIds); }

        std::tuple<float *, int, int> result = model->forward();
        float *outBuf = std::get<0>(result);
        int sampleOffset = std::get<1>(result);
        int sampleSize = std::get<2>(result);

        // Create a torch::Tensor from the C array
        int64_t tdims[3] = {batchSize, seqLen, vocabSize};
        torch::Tensor ret = torch::from_blob(outBuf, tdims, torch::kFloat32);
        return ret;
    }

    torch::Tensor generate() {
        auto nextTokens = model->generate();

        int batchSize = model->getBatchSize();
        int numBeams = model->getConfig().numBeams;

        torch::Tensor ret = torch::empty({batchSize, numBeams}, torch::kInt64);
        int64_t *p = ret.data_ptr<int64_t>();
        for (int i = 0; i < nextTokens.size(); ++i) {
            p[i] = nextTokens[i];
        }
        return ret;
    }

    torch::Tensor finalize() {
        auto outputs = model->finalize();

        int batchSize = model->getBatchSize();
        int numBeamHypsToKeep = model->getConfig().numBeamHypsToKeep;
        int outputLen = outputs.size() / (batchSize * numBeamHypsToKeep);

        torch::Tensor ret = torch::empty({batchSize * numBeamHypsToKeep, outputLen}, torch::kInt64);
        int64_t *p = ret.data_ptr<int64_t>();
        for (int i = 0; i < outputs.size(); ++i) {
            p[i] = outputs[i];
        }
        return ret;
    }

    void setPrefix(torch::optional<torch::Tensor> inputIds) {
        std::vector<int> prefixIds;
        if (model->getRank() == 0) {
            TORCH_CHECK(inputIds.has_value(), "Make sure master's prefix input is not None.")
            TORCH_CHECK(inputIds.value().dim() <= 2, "Prefix sharing input expected dim <= 2 but tensor has ",
                    inputIds.value().dim());
            inputIds.value().squeeze();
            TORCH_CHECK(inputIds.value().dim() == 2, "Prefix sharing only support 1 prompt but input has ",
                    inputIds.value().size(0));

            int seqLen = inputIds.value().size(-1);

            prefixIds.resize(seqLen);
            int64_t *p = inputIds.value().data_ptr<int64_t>();
            for (int i = 0; i < seqLen; ++i) {
                prefixIds[i] = static_cast<int>(*p);
                p += 1;
            }
        }

        model->setPrefix(prefixIds);
    };

    void unsetPrefix() { model->unsetPrefix(); };

private:
    xft::Model *model;
    std::vector<int> tokenIds;
};
