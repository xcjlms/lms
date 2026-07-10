import { createRouter, createWebHashHistory } from 'vue-router'

const routes = [
  { path: '/', redirect: '/login' },
  { path: '/login', name: 'Login', component: () => import('../views/Login.vue') },
  { path: '/home/reader', name: 'HomeReader', component: () => import('../views/HomeReader.vue') },
  { path: '/home/admin', name: 'HomeAdmin', component: () => import('../views/HomeAdmin.vue') },
  { path: '/borrow', name: 'Borrow', component: () => import('../views/Borrow.vue') },
  { path: '/back', name: 'Back', component: () => import('../views/Back.vue') },
  { path: '/fine', name: 'Fine', component: () => import('../views/Fine.vue') },
  { path: '/admin/books', name: 'BookManage', component: () => import('../views/BookManage.vue') },
  { path: '/admin/users', name: 'UserManage', component: () => import('../views/UserManage.vue') },
  { path: '/admin/feedback', name: 'FeedbackManage', component: () => import('../views/FeedbackManage.vue') },
  { path: '/admin/backup', name: 'Backup', component: () => import('../views/Backup.vue') },
  { path: '/reader/feedback', name: 'FeedbackSubmit', component: () => import('../views/FeedbackSubmit.vue') },
  { path: '/reader/profile', name: 'Profile', component: () => import('../views/Profile.vue') },
]

const router = createRouter({
  history: createWebHashHistory(),
  routes,
})

// 路由守卫：检查登录状态
router.beforeEach((to, from, next) => {
  const user = localStorage.getItem('lms_user')
  if (to.name !== 'Login' && !user) {
    next('/login')
  } else {
    next()
  }
})

export default router
