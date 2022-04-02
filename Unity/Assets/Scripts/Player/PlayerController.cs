using UnityEngine;
using System.Collections;
using UnityEngine.InputSystem;

[RequireComponent(typeof(Animator))]
[RequireComponent(typeof(Rigidbody))]
public class PlayerController : MonoBehaviour
{
	[Header("Jumping")]
	[SerializeField] private float m_JumpForce = 15.0f;
	[SerializeField] private float m_JumpForwardForceMultiplier = 5.0f;
	[SerializeField, Tooltip("Time before jump force is applied, giving time for an animation to begin")]
	private float m_JumpDelay = 0.5f;
	[SerializeField, Tooltip("Minimum amount of time on ground before character can jump again")]
	private float m_GroundedJumpDelay = 0.5f;

	[Header("Ground Check")]
	[SerializeField] private LayerMask m_GroundLayers;
	[SerializeField] private Vector3 m_GroundCastOrigin;
	[SerializeField] private float m_GroundCastLength = 2.0f;

	[SerializeField, Range(1, 6)]
	[Tooltip("How many casts offset around the center of transform")]
	private int m_GroundCastCount = 4;

	[SerializeField, Range(0.01f, 2.5f)]
	[Tooltip("Offset of rays, away from center of transform")]
	private float m_GroundCastOffset = 0.25f;

	[SerializeField, Range(0.0f, 360.0f)]
	[Tooltip("Rotation offset, in degrees")]
	private float m_GroundCastRotationOffset = 0.25f;

	[SerializeField, Tooltip("Minimum time, in seconds, to be off ground before animation triggers")]
	private float m_MinTimeBeforeFallAnim = 0.25f;

	[Header("Input")]
	[SerializeField, Tooltip("How much the player can influence velocity while in the air")]
	private float m_AirSpeed = 1.0f;

	[SerializeField, Tooltip("Maximum velocity magnitude while in the air")]
	private float m_MaxAirSpeed = 5.0f;
	
	[SerializeField] private float m_RotateSpeed = 1.0f;
	[SerializeField] private float m_TurnSmoothTime = 0.1f;
	[SerializeField] private InputActionReference m_JumpInput;
	[SerializeField] private InputActionReference m_SprintInput;
	[SerializeField] private InputActionReference m_MovementInput;

	[Header("Animation")]
	[SerializeField, Tooltip("Animation damping when switching inputs")]
	private float m_AnimationDamping = 0.05f;

	public bool IsGrounded { get; private set; }
	public bool CanJump => IsGrounded && m_GroundTime >= m_GroundedJumpDelay;
	public GroundInfo GroundInfo { get; private set; } = null;

	private Animator m_Animator;
	private Rigidbody m_Rigidbody;
	private float m_TurnSmoothVelocity;
	private Vector3 m_FloorPos = Vector3.zero;

	/// <summary>
	/// Accumulator used for checking how long character has been grounded.
	/// Used to check if character can jump again.
	/// </summary>
	private float m_GroundTime = 0.0f;
		
	private void Start()
	{
		m_Animator = GetComponent<Animator>();
		m_Rigidbody = GetComponent<Rigidbody>();

		m_Rigidbody.useGravity  = false;
		m_Rigidbody.isKinematic = false;
	}

	private void Update()
	{
		m_GroundTime += Time.deltaTime * (IsGrounded ? 1.0f : -1.0f);

		// Get input
		bool jumping = m_JumpInput.action.IsPressed() && IsGrounded && CanJump;
		bool sprinting = m_SprintInput.action.IsPressed();
		Vector2 movement = m_MovementInput.action.ReadValue<Vector2>();

		// Check for jumping
		if (jumping)
			StartCoroutine(BeginJump());

		RotatePlayer(movement);

		float movementAmount = Mathf.Clamp01(Mathf.Abs(movement.x) + Mathf.Abs(movement.y));

		if(sprinting)
			movementAmount *= 2.0f;

		// Update animator
		m_Animator.SetBool ("IsJumping",	jumping);
		m_Animator.SetBool ("IsSprinting",	sprinting);
		m_Animator.SetBool ("IsGrounded",	m_GroundTime >= -m_MinTimeBeforeFallAnim);
		m_Animator.SetBool ("IsMoving", movementAmount > 0.1f);
		m_Animator.SetFloat("Vertical Velocity", m_Rigidbody.velocity.y);
		m_Animator.SetFloat("Movement Input", movementAmount, m_AnimationDamping, Time.deltaTime);
	}

	private void RotatePlayer(Vector2 input)
	{
		Vector3 direction = new Vector3(input.x, 0, input.y).normalized;
		if(direction.sqrMagnitude < 0.01f)
			return; // No input

		Transform cam = Camera.main.transform;
		float targetAngle = Mathf.Atan2(direction.x, direction.z) * Mathf.Rad2Deg * m_RotateSpeed + cam.eulerAngles.y;
		float angle = Mathf.SmoothDampAngle(transform.eulerAngles.y, targetAngle, ref m_TurnSmoothVelocity, m_TurnSmoothTime);
		transform.rotation = Quaternion.Euler(0, angle, 0);
	}

	private void FixedUpdate()
	{
		CheckGrounded();

		if(IsGrounded)
		{
			Vector3 desiredPos = new Vector3(m_Rigidbody.position.x, m_FloorPos.y, m_Rigidbody.position.z);
			m_Rigidbody.position = Vector3.Lerp(m_Rigidbody.position, desiredPos, Time.deltaTime * 2.0f);
		}
		else
		{
			Vector2 movement = m_MovementInput.action.ReadValue<Vector2>();
			Vector3 airForce = transform.TransformDirection(new Vector3(movement.x, 0, movement.y) * m_AirSpeed);

			// Applied air movement
			m_Rigidbody.AddForce(airForce, ForceMode.Acceleration);

			// Clamp XZ air movement
			airForce = m_Rigidbody.velocity;
			float yVelocity = airForce.y;
			airForce.y = 0;
			m_Rigidbody.velocity = Vector3.ClampMagnitude(airForce, m_MaxAirSpeed) + new Vector3(0, yVelocity, 0);

			// Apply gravity
			m_Rigidbody.AddForce(Physics.gravity * Time.fixedDeltaTime, ForceMode.VelocityChange);
		}
	}

	private void CheckGrounded()
	{
		// Check for ground level
		uint hitFloors = 0;
		m_FloorPos = Vector3.zero;
		for (int i = 0; i < m_GroundCastCount; i++)
		{
			float angle = (i / (float)m_GroundCastCount) * Mathf.PI * 2.0f + m_GroundCastRotationOffset * Mathf.Deg2Rad;
			Vector3 rayOrigin = transform.position + m_GroundCastOrigin;
			rayOrigin.x += Mathf.Cos(angle) * m_GroundCastOffset;
			rayOrigin.z += Mathf.Sin(angle) * m_GroundCastOffset;
			if(!CheckGround(rayOrigin, out RaycastHit hit))
				continue;

			m_FloorPos += hit.point;
			hitFloors++;

			// Get ground sound info
			if(hit.collider.TryGetComponent(out GroundInfoHolder infoHolder))
				GroundInfo = infoHolder.Info;
		}

		bool grounded = hitFloors > 0;
		if(grounded)
			m_FloorPos /= hitFloors;
		else
			GroundInfo = null;

		if((grounded && !IsGrounded) || // Landed on ground
			(!grounded && IsGrounded)) // Leaving ground
		{
			m_Animator.applyRootMotion = grounded;
			m_GroundTime = 0;
		}

		IsGrounded = grounded;
	}

	[ExecuteInEditMode]
	private void OnDrawGizmos()
	{
		Gizmos.color = Color.red;
		Gizmos.DrawWireSphere(transform.position + m_GroundCastOrigin, 0.025f);
		Gizmos.color = IsGrounded ? Color.green : Color.red;
		for (int i = 0; i < m_GroundCastCount; i++)
		{
			float angle = (i / (float)m_GroundCastCount) * Mathf.PI * 2.0f + m_GroundCastRotationOffset * Mathf.Deg2Rad;
			Vector3 rayOrigin = transform.position + m_GroundCastOrigin;
			rayOrigin.x += Mathf.Cos(angle) * m_GroundCastOffset;
			rayOrigin.z += Mathf.Sin(angle) * m_GroundCastOffset;
			Gizmos.DrawRay(rayOrigin, Vector3.down * m_GroundCastLength);
		}
		if(IsGrounded)
			Gizmos.DrawWireSphere(m_FloorPos, 0.025f);
	}

	private IEnumerator BeginJump()
	{
		m_GroundTime = 0;
		m_Animator.applyRootMotion = false;

		// Wait for animation to start
		yield return new WaitForSeconds(m_JumpDelay);

		m_Rigidbody.AddForce(Vector3.up * m_JumpForce, ForceMode.Impulse);
		m_Rigidbody.velocity += m_Rigidbody.velocity * m_JumpForwardForceMultiplier;

		// Now off ground, reset the accumulator
		m_GroundTime = 0;
	}

	private bool CheckGround(Vector3 startPos, out RaycastHit hit) => Physics.Raycast(startPos, Vector3.down, out hit, m_GroundCastLength, m_GroundLayers);
}
