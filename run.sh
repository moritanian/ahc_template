#! /bin/bash
if [ -z "${input}" ]; then
    input="main.cpp"
fi

bin=a.out

# prod is defined, set DEBUG_OPTION="-DATCODERDEBUG"
DEBUG_OPTION=""
if [ -z "${prod:-}" ]; then
    DEBUG_OPTION="-DATCODERDEBUG"
fi

if ! g++-12 -std=gnu++2b $DEBUG_OPTION -Wall -Wextra -O2 -DONLINE_JUDGE -fconstexpr-depth=2147483647 \
    -fconstexpr-loop-limit=2147483647 -fconstexpr-ops-limit=2147483647 \
    -I/opt/boost/gcc/include -L/opt/boost/gcc/lib -I/home/moritanian/project/atcoder/lib/ac-library -static-libstdc++ \
    -o $bin "$input"; then
    exit 1
fi

#clang++ -std=c++2b -DATCODERDEBUG -Wall -Wextra -O2 -DONLINE_JUDGE -DATCODER -mtune=native -march=native -fconstexpr-depth=2147483647 -fconstexpr-steps=2147483647 \
#  -I/opt/boost/clang/include -L/opt/boost/clang/lib -I/opt/ac-library -I/usr/include/eigen3 -fuse-ld=lld -o a.out $input

echo "compile fin"
cd tools || (echo "tools not found" && exit 1)

SECONDS=0

CORE=$(lscpu | grep -E '^Core\(s\) per socket:' | awk '{print $4}')
SOCKETS=$(lscpu | grep -E '^Socket\(s\):' | awk '{print $2}')
PHYSICAL_CORES=$((CORE * SOCKETS))
NUM_PARALLEL=$PHYSICAL_CORES

ret=
score_sum=0
max_score=0
max_score_case=
cnt=0
elapsed_sum=0
elapsed_max=0
elapsed_max_case=

max_ratio=0
max_ratio_case=
ratio_sum=0

#rm -rf result/**
# rm -rf out/**

function run() {
    i=${1:3:4}

    _started_at=$(date +'%s.%3N')
    if ! bash -c "./../$bin <in/${i}.txt" 1>out/"${i}".txt 2>err/"${i}".txt; then
        # if ! bash -c "cargo run -r --bin tester ./../${bin} <in/${i}.txt >out/${i}.txt 2>err/${i}.txt"; then
        echo "Error" $i
        return
    fi
    # cargo run -r --bin tester ./../${bin} <in/${i}.txt >out/${i}.txt 2>result/${i}.txt
    #if ! sudo bash -c "perf record  ./../$bin <in/${i}.txt" 1>out/"${i}".txt 2>err/"${i}".txt; then
    #  echo "Error" $i
    #  return
    #fi

    _ended_at=$(date +'%s.%3N')

    #echo -e "$out" >out/${i}.txt

    _elapsed=$(echo "scale=3; $_ended_at - $_started_at" | bc)
    echo -e "Elapsed = $_elapsed" >>err/${i}.txt

}

function aggregate() {
    if [ ${1:-""} = "all" ]; then
        summary_file="summary.csv"
    else
        summary_file="summary_single.csv"
    fi

    echo "ID,SCORE,ELAPSED,update,M,L" >$summary_file

    echo "aggregate $1 $2"
    failed_cnt=0
    for i in $(seq $1 $2); do
        i="0000${i}"
        i=${i: -4}

        score=$(grep -oP 'Score = \K\d+' err/${i}.txt)
        elapsed=$(grep -oP 'Elapsed = \K\d*\.?\d+' err/${i}.txt)
        estimated=$(grep -oP 'EstimatedScore = \K\d+' out/${i}.txt)

        values=$(grep -oP '!D! \K.*' err/${i}.txt)
        values=$(echo "$values" | tr ' ' ',' | tr -d '\n' | sed 's/,$//')
        echo "$i,$score,$elapsed,$values" >>$summary_file

        re='^[0-9\.]+$'
        # count as failed if score is not a number or zero

        if [ -z "$score" ] || [ "$score" -eq 0 ]; then
            echo "WA " $i
            ((failed_cnt += 1))
            continue
        fi

        ratio=$estimated # TODO
        if [[ $ratio =~ $re ]]; then
            # echo "ratio" $ratio
            ratio_sum=$(echo "scale=3; $ratio_sum + $ratio" | bc)

            if [ "$(echo "$ratio > $max_ratio" | bc)" -eq 1 ]; then
                max_ratio=$ratio
                max_ratio_case=$i
            fi
            #else
            #echo "no ratio" $i
        fi

        score_sum=$((score_sum + score))

        if [ "$(echo "$score > $max_score" | bc)" -eq 1 ]; then
            max_score=$score
            max_score_case=$i
        fi

        if [ -n "$elapsed" ]; then
            elapsed_sum=$(echo "scale=3; $elapsed_sum + $elapsed" | bc)
            if [ "$(echo "scale=3; $elapsed > $elapsed_max" | bc)" -eq 1 ]; then
                elapsed_max=$elapsed
                elapsed_max_case=$i
            fi
        fi
    done

    average_ratio=$(echo "scale=3; $ratio_sum / ($cnt)" | bc)
    average_elapsed=$(echo "scale=3; $elapsed_sum / $cnt" | bc)

    echo num test = $cnt
    echo failed cnt = $failed_cnt

    echo -e "\n-------------- Ratio -----------------"
    echo "Max ratio = " $max_ratio " : " $max_ratio_case
    echo "Average ratio = " $average_ratio

    echo -e "\n------------- Elapsed Time --------------------"
    echo "average elapsed = " $average_elapsed "s"
    echo "max elapsed = " $elapsed_max " s : " $elapsed_max_case

    echo -e "\n-------------- Score -----------------"
    ave=$((score_sum / (cnt - failed_cnt)))
    echo "Average Score = " $ave

    echo "Max Score = " $max_score " : " $max_score_case
}

if [ ${1:-""} = "all" ]; then
    echo "NUM PARALLEL = " $NUM_PARALLEL
    files="in/*.txt"

    start=0
    end=0

    for filepath in $files; do

        i=${filepath:3:4}
        if [ -v 2 ] && [ $i -ge $2 ]; then
            continue
        fi

        run $filepath &

        pids[${cnt}]=$!
        ((cnt += 1))

        if [ $cnt -ge $NUM_PARALLEL ]; then
            wait ${pids[$((cnt - CORE))]}
        fi
        echo $cnt
        end=$i
    done

    # wait for all pids
    for pid in ${pids[*]}; do
        wait $pid
    done

    echo "aggregate ..."
    aggregate $start $end

elif [ ${1:-""} = "all_single" ]; then
    files="in/*.txt"
    for filepath in $files; do

        i=${filepath:3:4}
        if [ -v 2 ] && [ $i -ge $2 ]; then
            continue
        fi

        run $filepath
        echo $filepath $_elapsed

        ((cnt += 1))
    done

    aggregate
else
    v="0000${1:-}"
    i=${v: -4}
    filepath="in/$i.txt"
    run $filepath
    # echo $ret
    cnt=1

    aggregate $i $i
fi

echo -e "\nelapsed: $SECONDS"
