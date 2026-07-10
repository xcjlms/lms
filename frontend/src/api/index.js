// Unified API layer — all backend calls go through here
const BASE = '/api'

async function request(url, options = {}) {
  const config = {
    headers: { 'Content-Type': 'application/json' },
    ...options,
  }
  const res = await fetch(BASE + url, config)
  const data = await res.json()
  if (!data.success) {
    throw new Error(data.message || 'Request failed')
  }
  return data
}

// ---- Auth ----
export function login(username, password) {
  return request('/login', {
    method: 'POST',
    body: JSON.stringify({ username, password }),
  })
}

export function register(username, password, role) {
  return request('/register', {
    method: 'POST',
    body: JSON.stringify({ username, password, role }),
  })
}

// ---- Books ----
export function getBooks(keyword = '') {
  const query = keyword ? `?keyword=${encodeURIComponent(keyword)}` : ''
  return request('/books' + query)
}

export function addBook(book) {
  return request('/books', {
    method: 'POST',
    body: JSON.stringify(book),
  })
}

export function deleteBook(bookId) {
  return request(`/books/${bookId}`, { method: 'DELETE' })
}

export function editBook(bookId, book) {
  return request(`/books/${bookId}`, {
    method: 'PUT',
    body: JSON.stringify(book),
  })
}

// ---- Borrow / Return ----
export function borrowBook(userId, bookId) {
  return request('/borrow', {
    method: 'POST',
    body: JSON.stringify({ user_id: userId, book_id: bookId }),
  })
}

export function returnBook(userId, bookId) {
  return request('/return', {
    method: 'POST',
    body: JSON.stringify({ user_id: userId, book_id: bookId }),
  })
}

export function getBorrowHistory(userId) {
  return request(`/records?user_id=${userId}`)
}

// ---- Overdue ----
export function getOverdueRecords() {
  return request('/overdue')
}

// ---- Fines ----
export function getUserFines(userId) {
  return request(`/fines?user_id=${userId}`)
}

export function calculateFine(recordId) {
  return request('/fines/calculate', {
    method: 'POST',
    body: JSON.stringify({ record_id: recordId }),
  })
}

export function payFine(fineId) {
  return request('/fines/pay', {
    method: 'POST',
    body: JSON.stringify({ fine_id: fineId }),
  })
}

// ---- Inventory ----
export function getInventory() {
  return request('/inventory')
}

export function getLowStockBooks() {
  return request('/inventory/low')
}

export function increaseStock(bookId, quantity) {
  return request(`/books/${bookId}/stock/increase`, {
    method: 'POST',
    body: JSON.stringify({ quantity }),
  })
}

export function decreaseStock(bookId, quantity) {
  return request(`/books/${bookId}/stock/decrease`, {
    method: 'POST',
    body: JSON.stringify({ quantity }),
  })
}

// ---- Backup ----
export function backupData() {
  return request('/backup', { method: 'POST' })
}

export function restoreData(backupPath) {
  return request('/restore', {
    method: 'POST',
    body: JSON.stringify({ backup_path: backupPath }),
  })
}
// ---- Users ----
export function getUsers() {
  return request('/users')
}

export function updateUser(userId, role, status) {
  return request(`/users/${userId}`, {
    method: 'PUT',
    body: JSON.stringify({ role, status }),
  })
}

// ---- Feedback ----
export function submitFeedback(userId, title, content) {
  return request('/feedback', {
    method: 'POST',
    body: JSON.stringify({ user_id: userId, title, content }),
  })
}

export function getFeedbackList() {
  return request('/feedback')
}

export function replyFeedback(feedbackId, reply) {
  return request(`/feedback/${feedbackId}/reply`, {
    method: 'POST',
    body: JSON.stringify({ reply }),
  })
}

// ---- Profile ----
export function changePassword(userId, oldPassword, newPassword) {
  return request('/change_password', {
    method: 'POST',
    body: JSON.stringify({
      user_id: userId,
      old_password: oldPassword,
      new_password: newPassword,
    }),
  })
}
