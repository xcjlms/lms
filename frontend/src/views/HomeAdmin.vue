<template>
  <div class="home-admin">
    <aside class="side-menu">
      <div class="brand">图书管理后台</div>
      <nav>
        <ul>
          <li class="active">首页看板</li>
          <li><router-link to="/admin/books">图书管理</router-link></li>
          <li><router-link to="/admin/users">用户管理</router-link></li>
          <li><router-link to="/admin/feedback">反馈处理</router-link></li>
          <li><router-link to="/admin/backup">系统备份</router-link></li>
          <li @click="logout">退出登录</li>
          <li @click="logout">退出登录</li>
        </ul>
      </nav>
    </aside>

    <main class="dashboard-content">
      <section class="overview-cards">
        <article class="stat-card">
          <p class="title">在借图书总数</p>
          <p class="value">{{ borrowedTotal }}</p>
        </article>
        <article class="stat-card">
          <p class="title">逾期图书数量</p>
          <p class="value">{{ overdueCount }}</p>
        </article>
        <article class="stat-card">
          <p class="title">待缴罚款总额</p>
          <p class="value">¥{{ unpaidFines }}</p>
        </article>
        <article class="stat-card">
          <p class="title">馆藏图书总数</p>
          <p class="value">{{ totalBooks }}</p>
        </article>
      </section>

      <section class="charts-row">
        <article class="chart-card">
          <div class="chart-header">
            <h3>近 30 天借阅趋势</h3>
          </div>
          <div class="chart-placeholder">
            <div class="line-chart">
              <span class="point" style="--pos:10%"></span>
              <span class="point" style="--pos:30%"></span>
              <span class="point" style="--pos:55%"></span>
              <span class="point" style="--pos:40%"></span>
              <span class="point" style="--pos:80%"></span>
            </div>
          </div>
        </article>

        <article class="chart-card">
          <div class="chart-header">
            <h3>各分类图书馆藏占比</h3>
          </div>
          <div class="chart-placeholder pie-chart">
            <div class="slice slice-1"></div>
            <div class="slice slice-2"></div>
            <div class="slice slice-3"></div>
            <div class="slice slice-4"></div>
            <div class="pie-center">占比</div>
          </div>
          <ul class="legend-list">
            <li><span></span>文学</li>
            <li><span></span>科技</li>
            <li><span></span>历史</li>
            <li><span></span>少儿</li>
          </ul>
        </article>
      </section>

      <section class="tasks-section">
        <article class="task-card">
          <h4>逾期图书清单</h4>
          <p>12 条逾期记录，等待催还提醒</p>
          <button type="button">一键催还</button>
        </article>
        <article class="task-card">
          <h4>未支付罚款列表</h4>
          <p>8 名读者尚未结算罚金</p>
          <button type="button">查看详情</button>
        </article>
        <article class="task-card">
          <h4>未处理用户反馈</h4>
          <p>5 条新反馈待处理</p>
          <button type="button">前往处理</button>
        </article>
      </section>
    </main>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import * as api from '../api/index.js'

const router = useRouter()
const borrowedTotal = ref(0)
const overdueCount = ref(0)
const unpaidFines = ref(0)
const totalBooks = ref(0)

async function fetchData() {
  try {
    const bookRes = await api.getBooks()
    const books = bookRes.data?.books || []
    totalBooks.value = books.length
    borrowedTotal.value = books.reduce((s, b) => s + (b.total_stock - b.available_stock), 0)

    const overdueRes = await api.getOverdueRecords()
    const overdues = overdueRes.data || []
    overdueCount.value = overdues.length

    // Estimate unpaid fines from all users (simplified: query user 2 as sample)
    const fineRes = await api.getUserFines(2)
    const fines = fineRes.data?.fines || []
    unpaidFines.value = fines.reduce((s, f) => f.status === 'UNPAID' ? s + f.amount : s, 0)
  } catch (e) {
    console.error('Failed to load admin stats:', e)
  }
}

function logout() {
  localStorage.removeItem('lms_user')
  router.push('/login')
}

onMounted(fetchData)
</script>

<style scoped>
.home-admin {
  display: flex;
  min-height: 100vh;
  background: #f4f7fb;
  color: #2d3a4b;
}

.side-menu {
  width: 240px;
  background: #1f2a3a;
  color: #fff;
  padding: 24px 16px;
}

.brand {
  font-size: 18px;
  font-weight: 700;
  margin-bottom: 24px;
}

.side-menu nav ul {
  list-style: none;
  padding: 0;
  margin: 0;
}

.side-menu nav li {
  padding: 12px 16px;
  border-radius: 8px;
  margin-bottom: 8px;
  cursor: pointer;
  transition: background 0.2s ease;
}

.side-menu nav li.active,
.side-menu nav li:hover {
  background: rgba(255, 255, 255, 0.1);
}

.dashboard-content {
  flex: 1;
  padding: 24px;
}

.overview-cards {
  display: grid;
  grid-template-columns: repeat(4, minmax(0, 1fr));
  gap: 16px;
  margin-bottom: 24px;
}

.stat-card {
  background: #ffffff;
  border-radius: 16px;
  padding: 20px;
  box-shadow: 0 8px 20px rgba(20, 40, 60, 0.08);
}

.stat-card .title {
  font-size: 14px;
  color: #6b7a93;
  margin-bottom: 12px;
}

.stat-card .value {
  font-size: 32px;
  font-weight: 700;
  color: #1f2a3a;
}

.charts-row {
  display: grid;
  grid-template-columns: 2fr 1fr;
  gap: 16px;
  margin-bottom: 24px;
}

.chart-card {
  background: #fff;
  border-radius: 16px;
  padding: 20px;
  box-shadow: 0 8px 20px rgba(20, 40, 60, 0.08);
}

.chart-header h3 {
  font-size: 16px;
  margin: 0 0 18px;
}

.chart-placeholder {
  min-height: 300px;
  background: #f7f9fc;
  border-radius: 14px;
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
}

.line-chart {
  position: relative;
  width: 100%;
  height: 220px;
  display: flex;
  align-items: flex-end;
  justify-content: space-between;
  padding: 0 20px;
}

.line-chart::before {
  content: '';
  position: absolute;
  inset: 20px 0 20px 40px;
  border-left: 1px solid rgba(100, 116, 139, 0.28);
  border-bottom: 1px solid rgba(100, 116, 139, 0.28);
}

.line-chart .point {
  position: relative;
  width: 12px;
  height: 12px;
  border-radius: 50%;
  background: #4067f5;
  bottom: calc(var(--pos) * 1%);
}

.pie-chart {
  width: 220px;
  height: 220px;
  border-radius: 50%;
  position: relative;
}

.pie-chart .slice {
  position: absolute;
  inset: 0;
  border-radius: 50%;
}

.pie-chart .slice-1 { background: conic-gradient(#4f7dff 0 25%, transparent 0); }
.pie-chart .slice-2 { background: conic-gradient(#82c7ff 0 25%, transparent 0); transform: rotate(90deg); }
.pie-chart .slice-3 { background: conic-gradient(#ffd56b 0 20%, transparent 0); transform: rotate(180deg); }
.pie-chart .slice-4 { background: conic-gradient(#55d6a5 0 35%, transparent 0); transform: rotate(260deg); }
.pie-chart .pie-center {
  position: absolute;
  inset: 64px;
  background: #f4f7fb;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 14px;
  color: #4f5b73;
}

.legend-list {
  list-style: none;
  padding: 12px 0 0;
  margin: 0;
  display: grid;
  gap: 8px;
}

.legend-list li {
  display: flex;
  align-items: center;
  font-size: 14px;
  color: #4f5b73;
}

.legend-list span {
  width: 12px;
  height: 12px;
  border-radius: 3px;
  display: inline-block;
  margin-right: 10px;
}

.legend-list li:nth-child(1) span { background: #4f7dff; }
.legend-list li:nth-child(2) span { background: #82c7ff; }
.legend-list li:nth-child(3) span { background: #ffd56b; }
.legend-list li:nth-child(4) span { background: #55d6a5; }

.tasks-section {
  display: grid;
  grid-template-columns: repeat(3, minmax(0, 1fr));
  gap: 16px;
}

.task-card {
  background: #fff;
  border-radius: 16px;
  padding: 22px;
  box-shadow: 0 8px 20px rgba(20, 40, 60, 0.08);
}

.task-card h4 {
  margin: 0 0 12px;
  font-size: 16px;
}

.task-card p {
  color: #6b7a93;
  margin: 0 0 18px;
}

.task-card button {
  border: none;
  background: #4067f5;
  color: #fff;
  padding: 10px 16px;
  border-radius: 10px;
  cursor: pointer;
}

.task-card button:hover {
  background: #3454c4;
}
</style>
