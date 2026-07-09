<template>
  <div class="home-reader">
    <header class="topbar">
      <div class="brand">图书馆读者端</div>
      <div class="topbar-actions">
        <div class="alert">
          <span class="dot" v-if="hasOverdue"></span>
          逾期提醒
        </div>
        <div class="profile">
          <div class="avatar">{{ user.avatarText }}</div>
          <div class="user-info">
            <div class="name">{{ user.name }}</div>
            <div class="role">读者</div>
          </div>
          <button class="logout">退出登录</button>
        </div>
      </div>
    </header>

    <div class="layout">
      <aside class="sidebar">
        <nav>
          <ul>
            <li v-for="item in menu" :key="item">{{ item }}</li>
          </ul>
        </nav>
      </aside>

      <main class="content">
        <section class="banner">
          <div class="banner-title">新书推荐</div>
          <div class="carousel">
            <article v-for="book in newBooks" :key="book.id" class="banner-card">
              <div class="cover">{{ book.tag }}</div>
              <div class="info">
                <h3>{{ book.title }}</h3>
                <p>{{ book.author }}</p>
              </div>
            </article>
          </div>
        </section>

        <section class="warning-card">
          <div class="warning-header">重要提醒</div>
          <div class="warning-content">
            <div>当前逾期图书：<strong>{{ stats.overdueBooks }} 本</strong></div>
            <div>待缴罚款金额：<strong>{{ stats.fines }} 元</strong></div>
          </div>
        </section>

        <section class="stats-cards">
          <article class="stat-card">
            <div class="stat-value">{{ stats.borrowed }}</div>
            <div class="stat-label">已借图书</div>
          </article>
          <article class="stat-card">
            <div class="stat-value">{{ stats.remaining }}</div>
            <div class="stat-label">剩余可借</div>
          </article>
          <article class="stat-card">
            <div class="stat-value">{{ stats.historyTotal }}</div>
            <div class="stat-label">历史借阅</div>
          </article>
        </section>

        <section class="hot-books">
          <div class="section-header">热门图书</div>
          <div class="hot-scroll">
            <article v-for="book in hotBooks" :key="book.id" class="book-card">
              <div class="cover"></div>
              <div class="book-title">{{ book.title }}</div>
              <div class="book-meta">{{ book.author }}</div>
              <div class="stock">可借库存：{{ book.stock }}</div>
            </article>
          </div>
        </section>

        <section class="category-entry">
          <div class="section-header">分类快捷入口</div>
          <div class="category-grid">
            <button v-for="category in categories" :key="category.name" class="category-button" :style="{ backgroundColor: category.color }">
              {{ category.name }}
            </button>
          </div>
        </section>
      </main>
    </div>
  </div>
</template>

<script setup>
import { reactive } from 'vue'

const user = reactive({
  name: '张三',
  avatarText: '张',
})

const menu = [
  '首页看板',
  '图书检索',
  '分类浏览',
  '我的借阅',
  '我的罚款',
  '意见反馈',
  '个人信息修改'
]

const hasOverdue = true

const newBooks = [
  { id: 1, title: '未来简史', author: '尤瓦尔·赫拉利', tag: '新书' },
  { id: 2, title: '活着', author: '余华', tag: '推荐' },
  { id: 3, title: '时间简史', author: '史蒂芬·霍金', tag: '热门' }
]

const stats = reactive({
  overdueBooks: 1,
  fines: 35,
  borrowed: 3,
  remaining: 2,
  historyTotal: 18
})

const hotBooks = [
  { id: 11, title: '围城', author: '钱钟书', stock: 7 },
  { id: 12, title: '三体', author: '刘慈欣', stock: 5 },
  { id: 13, title: '白夜行', author: '东野圭吾', stock: 6 },
  { id: 14, title: '百年孤独', author: '加西亚·马尔克斯', stock: 4 },
  { id: 15, title: '人类简史', author: '尤瓦尔·赫拉利', stock: 8 }
]

const categories = [
  { name: '文学', color: '#ff8a80' },
  { name: '科技', color: '#80d8ff' },
  { name: '历史', color: '#ffd180' },
  { name: '教育', color: '#b9f6ca' }
]
</script>

<style scoped>
.home-reader { min-height: 100vh; font-family: "Helvetica Neue", Arial, sans-serif; background: #f5f7fb; color: #222; }
.topbar { display: flex; justify-content: space-between; align-items: center; padding: 18px 24px; background: #fff; border-bottom: 1px solid #ebedf0; box-shadow: 0 1px 4px rgba(0,0,0,0.04); position: sticky; top: 0; z-index: 10; }
.brand { font-size: 18px; font-weight: 700; }
.topbar-actions { display: flex; align-items: center; gap: 18px; }
.alert { position: relative; padding: 6px 12px; border: 1px solid #ff6b6b; border-radius: 18px; color: #d32f2f; font-weight: 500; }
.alert .dot { position: absolute; top: 6px; right: 6px; width: 8px; height: 8px; border-radius: 50%; background: #d32f2f; }
.profile { display: flex; align-items: center; gap: 12px; }
.avatar { width: 40px; height: 40px; border-radius: 50%; background: #5c6bc0; color: #fff; display: grid; place-items: center; font-weight: 700; }
.user-info { text-align: right; line-height: 1.1; }
.user-info .name { font-weight: 600; }
.logout { border: 1px solid #dcdcdc; background: #fff; padding: 6px 12px; border-radius: 4px; cursor: pointer; }
.layout { display: flex; padding: 24px; gap: 24px; }
.sidebar { width: 220px; background: #fff; border: 1px solid #ebedf0; border-radius: 14px; padding: 20px 0; flex-shrink: 0; }
.sidebar nav ul { list-style: none; margin: 0; padding: 0; }
.sidebar nav li { padding: 14px 24px; cursor: pointer; color: #333; transition: background .2s ease; }
.sidebar nav li:hover { background: #f5f7fb; }
.content { flex: 1; display: flex; flex-direction: column; gap: 20px; }
.banner { background: linear-gradient(135deg, #5164f5 0%, #8f9dff 100%); border-radius: 20px; color: #fff; padding: 24px; }
.banner-title { font-size: 18px; font-weight: 600; margin-bottom: 16px; }
.carousel { display: flex; gap: 16px; overflow-x: auto; padding-bottom: 8px; }
.banner-card { min-width: 220px; background: rgba(255,255,255,.16); border-radius: 18px; padding: 18px; flex-shrink: 0; box-shadow: 0 8px 20px rgba(0,0,0,0.08); }
.banner-card .cover { width: 100%; height: 120px; border-radius: 14px; background: rgba(255,255,255,.3); display: grid; place-items: center; font-size: 20px; margin-bottom: 14px; }
.warning-card { border: 1px solid #f44336; background: #fff6f6; border-radius: 16px; padding: 20px; }
.warning-header { color: #c62828; font-weight: 700; margin-bottom: 14px; }
.warning-content { display: flex; gap: 40px; flex-wrap: wrap; }
.warning-content div { min-width: 240px; font-size: 15px; }
.stats-cards { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 16px; }
.stat-card { background: #fff; border-radius: 16px; padding: 20px; box-shadow: 0 10px 24px rgba(18, 38, 63, 0.05); }
.stat-value { font-size: 32px; font-weight: 700; margin-bottom: 8px; }
.stat-label { color: #666; }
.hot-books { background: #fff; border-radius: 20px; padding: 20px; }
.section-header { font-size: 18px; font-weight: 700; margin-bottom: 16px; }
.hot-scroll { display: flex; gap: 16px; overflow-x: auto; padding-bottom: 8px; }
.book-card { min-width: 160px; background: #f9fbff; border-radius: 18px; padding: 16px; box-shadow: 0 10px 24px rgba(32, 48, 72, 0.06); }
.book-card .cover { width: 100%; height: 180px; border-radius: 12px; background: linear-gradient(135deg, #819cff, #aac9ff); margin-bottom: 12px; }
.book-title { font-size: 15px; font-weight: 600; margin-bottom: 6px; }
.book-meta { color: #666; font-size: 13px; margin-bottom: 10px; }
.stock { font-size: 13px; color: #2e7d32; font-weight: 500; }
.category-entry { background: #fff; border-radius: 20px; padding: 20px; }
.category-grid { display: grid; grid-template-columns: repeat(4, minmax(0, 1fr)); gap: 16px; }
.category-button { border: none; border-radius: 16px; padding: 18px 0; color: #fff; font-weight: 600; cursor: pointer; transition: transform .2s ease; }
.category-button:hover { transform: translateY(-2px); }
@media (max-width: 1200px) {
  .layout { flex-direction: column; }
  .sidebar { width: 100%; }
  .stats-cards { grid-template-columns: repeat(2, minmax(0, 1fr)); }
  .category-grid { grid-template-columns: repeat(2, minmax(0, 1fr)); }
}
@media (max-width: 768px) {
  .topbar { flex-wrap: wrap; gap: 12px; }
  .layout { padding: 16px; }
  .stats-cards { grid-template-columns: 1fr; }
  .hot-scroll, .carousel { overflow-x: auto; }
  .category-grid { grid-template-columns: 1fr; }
}
</style>
