

### Prepare xFT envs

``` bash
$ conda create -n xft python=3.10
$ conda activate xft

$ git clone https://github.com/intel/xFasterTransformer.git && cd xFasterTransformer
$ pip3 install -r requirements.txt
$ bash ci_build build

$ python setup.py build
$ python setup.py bdist_wheel --verbose 
# whl package will be created under ./dist folder
```

### Prepare opencompass envs
``` bash
# refer to https://opencompass.org.cn/doc
$ conda create -n opencompass python=3.10 pytorch torchvision torchaudio cpuonly -c pytorch -y
$ conda activate opencompass

$ git clone -b intel/xft https://github.com/marvin-Yu/opencompass.git && cd opencompass
$ pip install -e .

# install xFT whl package
$ pip install <xFT package>

# download core dataset
$ wget https://github.com/open-compass/opencompass/releases/download/0.2.2.rc1/OpenCompassData-core-20240207.zip
$ unzip OpenCompassData-core-20240207.zip

# # download full dataset
# $ wget https://github.com/open-compass/opencompass/releases/download/0.2.2.rc1/OpenCompassData-complete-20240207.zip
# $ unzip OpenCompassData-complete-20240207.zip
# $ cd ./data
# $ find . -name "*.zip" -exec unzip "{}" \;

# list all xFT support models
$ python tools/list_configs.py xft
# +------------------------+----------------------------------------------+
# | Model                  | Config Path                                  |
# |------------------------+----------------------------------------------|
# | xft_baichuan2_13b_chat | configs/models/xft/xft_baichuan2_13b_chat.py |
# | xft_baichuan2_7b_chat  | configs/models/xft/xft_baichuan2_7b_chat.py  |
# | xft_llama2_13b_chat    | configs/models/xft/xft_llama2_13b_chat.py    |
# | xft_llama2_70b_chat    | configs/models/xft/xft_llama2_70b_chat.py    |
# | xft_llama2_7b_chat     | configs/models/xft/xft_llama2_7b_chat.py     |
# | xft_chatglm2_6b        | configs/models/xft/xft_chatglm2_6b.py        |
# | xft_chatglm3_6b        | configs/models/xft/xft_chatglm3_6b.py        |
# | xft_chatglm_6b         | configs/models/xft/xft_chatglm_6b.py         |
# | xft_gemma_2b_it        | configs/models/xft/xft_gemma_2b_it.py        |
# | xft_gemma_7b_it        | configs/models/xft/xft_gemma_7b_it.py        |
# +------------------------+----------------------------------------------+

# list dataset than you want.
$ python tools/list_configs.py ceval
# +--------------------------------+------------------------------------------------------------------+
# | Dataset                        | Config Path                                                      |
# |--------------------------------+------------------------------------------------------------------|
# | ceval_clean_ppl                | configs/datasets/ceval/ceval_clean_ppl.py                        |
# | ceval_contamination_ppl_810ec6 | configs/datasets/contamination/ceval_contamination_ppl_810ec6.py |
# | ceval_gen                      | configs/datasets/ceval/ceval_gen.py                              |
# | ceval_gen_2daf24               | configs/datasets/ceval/ceval_gen_2daf24.py                       |
# | ceval_gen_5f30c7               | configs/datasets/ceval/ceval_gen_5f30c7.py                       |
# | ceval_internal_ppl_1cd8bf      | configs/datasets/ceval/ceval_internal_ppl_1cd8bf.py              |
# | ceval_ppl                      | configs/datasets/ceval/ceval_ppl.py                              |
# | ceval_ppl_1cd8bf               | configs/datasets/ceval/ceval_ppl_1cd8bf.py                       |
# | ceval_ppl_578f8d               | configs/datasets/ceval/ceval_ppl_578f8d.py                       |
# | ceval_ppl_93e5ce               | configs/datasets/ceval/ceval_ppl_93e5ce.py                       |
# | ceval_zero_shot_gen_bd40ef     | configs/datasets/ceval/ceval_zero_shot_gen_bd40ef.py             |
# +--------------------------------+------------------------------------------------------------------+

# run eval test
$ python run.py --models xft_gemma_2b_it xft_llama2_7b_chat --datasets ceval_gen ceval_ppl
# 20240416_100621
# tabulate format
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# dataset                                         version    metric         mode      gemma-2b-it-hf-bf16    gemma-2b-it-hf-xft-fp16    gemma-2b-it-hf-xft-bf16
# ----------------------------------------------  ---------  -------------  ------  ---------------------  -------------------------  -------------------------
# ceval-computer_network                          db9ce2     accuracy       gen                     47.37                      47.37                      47.37
# ceval-operating_system                          1c2571     accuracy       gen                     42.11                      36.84                      36.84
# ceval-computer_architecture                     a74dad     accuracy       gen                     23.81                      28.57                      28.57
# ceval-college_programming                       4ca32a     accuracy       gen                     37.84                      37.84                      37.84
# ceval-college_physics                           963fa8     accuracy       gen                     36.84                      36.84                      36.84
# ceval-college_chemistry                         e78857     accuracy       gen                     37.5                       37.5                       37.5
# ceval-advanced_mathematics                      ce03e2     accuracy       gen                     10.53                      10.53                      10.53
# ...
```