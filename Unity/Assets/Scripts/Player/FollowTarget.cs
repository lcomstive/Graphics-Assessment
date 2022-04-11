using UnityEngine;
using UnityEngine.InputSystem;

public class FollowTarget : MonoBehaviour
{
	[SerializeField] private Transform m_Target;
	[SerializeField] private Vector3 m_Offset;
	[SerializeField] private float m_RotationSpeed = 1.0f;

	[SerializeField] private float m_FollowSpeed = 2.0f;

	[Header("Input")]
	[SerializeField] private InputActionReference m_LookAction;
	[SerializeField] private float m_LookSensitivity = 1.0f;

	[SerializeField, Tooltip("How long to wait for the mouse to stop moving before centering on the target")]
	private float m_CenterTimeNoInput = 2.5f;

	private float m_NoInputTime = 0.0f;

	private void Update()
	{
		// Position
		transform.position = m_Target.position + m_Offset;

		// Rotation
		Vector2 input = m_LookAction.action.ReadValue<Vector2>();
		if(input.sqrMagnitude > 0.1f && Cursor.lockState == CursorLockMode.Locked)
		{
			transform.eulerAngles += new Vector3(-input.y, input.x, 0) * m_LookSensitivity * Time.deltaTime;
			m_NoInputTime = 0.0f;
		}
		else
		{
			// No input received
			m_NoInputTime += Time.deltaTime;
			if(m_NoInputTime >= m_CenterTimeNoInput)
				transform.rotation = Quaternion.Slerp(transform.rotation, m_Target.rotation, Time.deltaTime * m_RotationSpeed);
		}
	}

	private void OnDrawGizmos()
	{
		Gizmos.color = Color.blue;
		Gizmos.DrawWireSphere(m_Target.position + m_Offset, 0.05f);
	}
}
