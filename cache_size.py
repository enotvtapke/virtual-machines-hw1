import pandas as pd
import ruptures as rpt

df = pd.read_csv('./cmake-build-debug/cache_assoc_table.csv')
data = df.to_numpy()
jumps_per_stride: list[list[int]] = []
assocs: list[int] = []
strides: list[int] = []
for row in data:
    stride = row[0]
    strides.append(int(stride))
    measurements = row[1:]
    algo = rpt.Pelt(model="l1", min_size=4, jump=1).fit(measurements)
    result = algo.predict(pen=40)
    assocs = result
    result.pop()
    jumps_per_stride.append(result)

if __name__ == '__main__':
    for assoc in assocs:
        i: int = 0
        for stride, jumps in reversed(list(zip(strides, jumps_per_stride))):
            if all(jump != assoc for jump in jumps) and any((assoc * 1.8 < jump < assoc * 2.2) for jump in jumps):
                print(f'Entity with assoc {assoc} has entity stride {stride * 2} Bytes and size {stride * 2 * assoc} Bytes.')
                break
