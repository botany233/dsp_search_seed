import importlib
import json
import multiprocessing as mp
import random
import sys
import traceback
from collections import deque
from pathlib import Path
from queue import Empty

from tqdm import tqdm


PROJECT_ROOT = Path(__file__).resolve().parent.parent

def _worker(api_name, task_queue, result_queue):
    task = None
    try:
        # 两个扩展的命名空间冲突，只在各自的子进程中导入。
        sys.path.insert(0, str(PROJECT_ROOT))
        api = importlib.import_module(api_name)
        api.set_device_id_c(-1)
        while True:
            task = task_queue.get()
            if task is None:
                return
            task_id, seed_id, star_num, resource_index, quick = task
            seed = api.Seed(seed_id, star_num, resource_index)
            galaxy_data = api.search_seed.get_galaxy_data_c(seed, quick)
            galaxy_json = json.dumps(
                api.data_to_dict(galaxy_data),
                ensure_ascii=True,
                sort_keys=True,
                separators=(",", ":"),
            )
            result_queue.put((task_id, galaxy_json))
    except Exception:
        result_queue.put({
            "api_name": api_name,
            "task": task,
            "traceback": traceback.format_exc(),
        })


def _receive_result(result_queue, worker):
    while True:
        try:
            result = result_queue.get(timeout=1.0)
        except Empty:
            if worker.exitcode is not None:
                raise RuntimeError(
                    f"{worker.name} exited before returning a result "
                    f"(exit code {worker.exitcode})"
                )
            continue
        if isinstance(result, dict):
            raise RuntimeError(
                f"{result['api_name']} failed for task {result['task']}:\n"
                f"{result['traceback']}"
            )
        return result


if __name__ == "__main__":
    quick = bool(1)
    test_num = 100000 if quick else 1000
    queue_size = 256 if quick else 16

    context = mp.get_context("spawn")
    api_names = ("CApi", "CApi_old")
    task_queues = [context.Queue(maxsize=queue_size) for _ in api_names]
    result_queues = [context.Queue(maxsize=queue_size) for _ in api_names]
    workers = [context.Process(target=_worker, name=api_name, args=(api_name, tasks, results)) for api_name, tasks, results in zip(api_names, task_queues, result_queues)]
    pending = deque()
    submitted = 0
    try:
        for worker in workers:
            worker.start()
        with tqdm(total=test_num) as progress:
            while submitted < test_num or pending:
                # 最多八组在途任务；比较完成一组之后才补充任务。
                while submitted < test_num and len(pending) < queue_size:
                    task = (
                        submitted,
                        random.randint(0, 99999999),
                        random.randint(32, 64),
                        random.randint(0, 10),
                        quick,
                    )
                    for task_queue in task_queues:
                        task_queue.put(task)
                    pending.append(task)
                    submitted += 1

                task_id, seed_id, star_num, resource_index, quick = pending.popleft()
                results = [_receive_result(result_queue, worker) for result_queue, worker in zip(result_queues, workers)]
                for api_name, (result_id, _) in zip(api_names, results):
                    if result_id != task_id:
                        raise RuntimeError(f"{api_name} returned task {result_id}, expected {task_id}")
                capi_json, old_capi_json = (result[1] for result in results)
                if capi_json != old_capi_json:
                    case_name = f"seed_{seed_id}_{star_num}_{resource_index}_{quick}"
                    output_paths = []
                    for api_name, galaxy_json in zip(api_names, (capi_json, old_capi_json)):
                        output_path = Path(__file__).resolve().parent / f"{case_name}_{api_name}.json"
                        formatted_json = json.dumps(json.loads(galaxy_json), ensure_ascii=False, indent=4)
                        output_path.write_text(f"{formatted_json}\n", encoding="utf-8")
                        output_paths.append(output_path)
                    raise AssertionError(
                        f"CApi mismatch for Seed({seed_id}, {star_num}, {resource_index}), "
                        f"quick={quick}; results saved to {output_paths[0]} and {output_paths[1]}"
                    )
                progress.update(1)

        for task_queue in task_queues:
            task_queue.put(None)
        for worker in workers:
            worker.join(timeout=5)
            if worker.is_alive() or worker.exitcode != 0:
                raise RuntimeError(f"{worker.name} failed to shut down (exit code {worker.exitcode})")
    finally:
        for worker in workers:
            if worker.is_alive():
                worker.terminate()
        for worker in workers:
            if worker.pid is not None:
                worker.join()
                worker.close()
        for queue in task_queues + result_queues:
            # 异常退出时可能还有未消费任务，不等待 feeder 刷入无人读取的管道。
            queue.cancel_join_thread()
            queue.close()
