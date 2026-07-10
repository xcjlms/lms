<template>
  <div class="feedback-manage">
    <h2>反馈处理</h2>
    <div v-if="loading">加载中…</div>
    <div v-else>
      <div v-for="item in list" :key="item.feedback_id" class="feedback-item">
        <div class="header">
          <span class="user">用户 #{{ item.user_id }}</span>
          <span class="time">{{ item.submitted_at }}</span>
          <span :class="['status', item.status === 'PENDING' ? 'pending' : 'addressed']">
            {{ item.status === 'PENDING' ? '待处理' : '已处理' }}
          </span>
        </div>
        <div class="title">{{ item.title }}</div>
        <div class="content">{{ item.content }}</div>
        <div v-if="item.reply" class="reply">回复：{{ item.reply }}</div>
        <div v-if="item.status === 'PENDING'" class="reply-form">
          <input v-model="replies[item.feedback_id]" placeholder="输入回复内容" />
          <button @click="doReply(item.feedback_id)">回复</button>
        </div>
      </div>
      <div v-if="list.length === 0">暂无反馈</div>
    </div>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted } from 'vue'
import * as api from '../api/index.js'

const list = ref([])
const loading = ref(false)
const replies = reactive({})

async function fetchList() {
  loading.value = true
  try {
    const res = await api.getFeedbackList()
    list.value = res.data?.list || []
  } catch (e) {
    alert('加载失败：' + e.message)
  } finally {
    loading.value = false
  }
}

async function doReply(id) {
  const reply = replies[id]
  if (!reply) return
  try {
    await api.replyFeedback(id, reply)
    delete replies[id]
    await fetchList()
  } catch (e) {
    alert('回复失败：' + e.message)
  }
}

onMounted(fetchList)
</script>

<style scoped>
.feedback-manage { padding: 20px; }
.feedback-item { border: 1px solid #ebeef5; border-radius: 8px; padding: 16px; margin-bottom: 16px; }
.header { display: flex; gap: 16px; align-items: center; margin-bottom: 8px; font-size: 14px; }
.status { padding: 2px 10px; border-radius: 12px; font-size: 12px; }
.status.pending { background: #fff0e0; color: #e67e22; }
.status.addressed { background: #e8f5e9; color: #2e7d32; }
.title { font-weight: 600; font-size: 16px; margin-bottom: 6px; }
.content { color: #555; margin-bottom: 8px; }
.reply { background: #f5f7fa; padding: 8px 12px; border-radius: 4px; margin-bottom: 8px; }
.reply-form { display: flex; gap: 8px; margin-top: 8px; }
.reply-form input { flex: 1; padding: 6px 10px; border: 1px solid #dcdfe6; border-radius: 4px; }
</style>
