#!/bin/fish

# 结束之前的服务端运行（如果之前未正常结束）
set server_pid $(ps aux | grep './MuelsyseTestServer' | grep -v grep | awk '{print $2}')

if test -n "$server_pid"
    kill -9 $server_pid
end

function build_project
    set project $argv[1]
    cd $project
	if not test -d build
		mkdir build
	end
    cd build
	if not test -f CMakeCache.txt
		cmake ../ -DCMAKE_BUILD_TYPE=Debug
	end
    make -j8
    if test $project = server
        nohup ./MuelsyseTestServer > /dev/null 2>&1 &
    else
        ./MuelsyseTestClient
    end
    cd ../..
end

# 构建项目并运行
build_project server
build_project client

# 结束本次运行的服务端
kill -9 $(ps aux | grep './MuelsyseTestServer' | grep -v grep | awk '{print $2}')

set COVERAGE_FILE coverage.info
set REPORT_FOLDER coverage_report

lcov --rc lcov_branch_coverage=1 -c -d client/build -o {$COVERAGE_FILE}_tmp
lcov --rc lcov_branch_coverage=1 -e {$COVERAGE_FILE}_tmp "*src*" -o {$COVERAGE_FILE}
genhtml --rc genhtml_branch_coverage=1 {$COVERAGE_FILE} -o {$REPORT_FOLDER}
rm -rf {$COVERAGE_FILE}_tmp
rm -rf {$COVERAGE_FILE}
